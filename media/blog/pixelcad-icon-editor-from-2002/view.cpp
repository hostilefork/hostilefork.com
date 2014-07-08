#include "stdafx.h"
#include "wfx.h"
#include "view.h"
#include "doc.h"
#include "rc/rsrc.h"

using namespace Gdiplus; // we do a lot of GDI+ calls in the view

#include <math.h>
#include <shellapi.h>

CR CrDarker(CR cr, int percent)
	{
	int r = cr.GetR() - cr.GetR() * (static_cast<float>(percent) / 100);
	int g = cr.GetG() - cr.GetG() * (static_cast<float>(percent) / 100);
	int b = cr.GetB() - cr.GetB() * (static_cast<float>(percent) / 100);
	if (r < 0)
		r = 0;
	if (g < 0)
		g = 0;
	if (b < 0)
		b = 0;
	return CR (255, r, g, b);
	}
	
CR CrLighter(CR cr, int percent)
	{
	int r = cr.GetR() + cr.GetR() * (static_cast<float>(percent) / 100);
	int g = cr.GetG() + cr.GetG() * (static_cast<float>(percent) / 100);
	int b = cr.GetB() + cr.GetB() * (static_cast<float>(percent) / 100);
	if (r > 255)
		r = 255;
	if (g > 255)
		g = 255;
	if (b > 255)
		b = 255;
	return CR (255, r, g, b);
	}


// create derived graphics class off of GDI+ graphics with a few more features we need in this program...
class DrawGraphics : public Graphics
	{
private:
	Font _fontTab;
	Font _fontTabBold;
	
public:
	DrawGraphics::DrawGraphics (HDC hdc) :
		Graphics (hdc),
		_fontTab (L"System", 8, FontStyleRegular),
		_fontTabBold (L"System", 8, FontStyleBold)
		{
		}
	
	void DrawTriangle(Color cr, PT ptTopLeft, DYP dypHeight)
		{
		PointF rgPoint[3];
		rgPoint[0] = PointF(ptTopLeft.Xp(), ptTopLeft.Yp());
		rgPoint[1] = PointF(ptTopLeft.Xp() + dypHeight, ptTopLeft.Yp());
		rgPoint[2] = PointF(ptTopLeft.Xp(), ptTopLeft.Yp() + dypHeight);
		
		SolidBrush brushFill (Color());
		GdiplusVerify(_d_, this->FillPolygon(&brushFill, rgPoint, Length(rgPoint)));
		
		Pen penOutline (Color(0, 0, 0));
		GdiplusVerify(_d_, this->DrawPolygon(&penOutline, rgPoint, Length(rgPoint)));
		}

	static const DXP dxpTab = 100;
	static const DYP dypTab = 16;
	
	void DrawTabBar(RC rc, ILAYER ilayer)
		{
		}
		
	void DrawTab(STR str, RC rc, F fCurrent, F fPressed, F fHover)
		{
		const CR crTabNormal (224, 224, 244);
		const CR crTabSelected (crLtGray);
		
		SolidBrush brushDraw (fCurrent ? crTabSelected : crTabNormal);
		GdiplusVerify(_d_, this->FillRectangle(&brushDraw, RectF(rc.XpLeft(), rc.YpTop(), rc.DxpWidth(), rc.DypHeight())));
		
		// Create a string.
		WCHAR wsz[256];
		Verify(_d_, CchAszToWsz(PszcOfStr(str), wsz, Length(wsz)) < Length(wsz));

		// Initialize arguments.
		PointF origin (rc.XpLeft(), rc.YpTop());
		SolidBrush brushText(fHover || fPressed ? crWhite : (fCurrent ? crBlack : crGray));

		// Draw string.
		this->DrawString(wsz, CchOfWsz(wsz), fCurrent ? &_fontTabBold : &_fontTab, origin, &brushText);
		
		if (fPressed)
			{
			Pen penPress (crBlack);
			penPress.SetDashStyle(DashStyleDash);
			penPress.SetDashCap(DashCapRound);
			this->DrawRectangle(&penPress, RectF(rc.XpLeft() + 2, rc.YpTop() + 2, rc.DxpWidth() - 4, rc.DypHeight() - 4));
			}
			
		Pen penLine (fCurrent ? crBlack : crGray);
		this->DrawLine(&penLine, rc.XpRight() - 1, rc.YpTop() + 1, rc.XpRight() - 1, rc.YpBottom() - (fCurrent ? 0 : 2));
		}
	};


// HIT prototypes





// HIT_LINE_SEL

VIEW_DRAW::HIT_LINE_SEL::HIT_LINE_SEL (XP xpStart, YP ypStart) :
	HIT_LINE<DOC_DRAW, VIEW_DRAW> (),
	_xpStart (xpStart),
	_ypStart (ypStart)
	{
	_hcursor = LoadCursor(null, IDC_ARROW);
	}

bool VIEW_DRAW::HIT_LINE_SEL::operator==(const HIT& rhs) const
	{
	const HIT_LINE_SEL* phit_line_selNew = reinterpret_cast<const HIT_LINE_SEL*>(&rhs);
	if ((_xpStart == phit_line_selNew->_xpStart) && (_ypStart == phit_line_selNew->_ypStart))
		return fTrue;
	return fFalse; // not the "same" selection, e.g. part of the same enterhover/exithover group
	}

HCURSOR VIEW_DRAW::HIT_LINE_SEL::Hcursor() const
	{
	return _hcursor;
	}


VIEW_DRAW::DO_DROPBITMAP::~DO_DROPBITMAP ()
	{
	delete _rgrgcr;
	}
VIEW_DRAW::DO_DROPBITMAP::DO_DROPBITMAP (HBITMAP hbitmap, const LAYER* player, XP xpCenter, YP ypCenter)
	{
	P<Bitmap> pbitmap (Bitmap::FromHBITMAP(hbitmap, null));
	InitializeHelper(pbitmap.get(), player, xpCenter, ypCenter);
	}
VIEW_DRAW::DO_DROPBITMAP::DO_DROPBITMAP (STR strFile, const LAYER* player, XP xpCenter, YP ypCenter) :
	DO_LINE<DOC_DRAW, VIEW_DRAW> (),
	_strFile (strFile),
	_player (player)
	{
	const DOC_DRAW* pdoc = PdocOfDo();
	
	// theoretically we should put this in idle, I guess (?)
	WCHAR wsz[256];
	Verify(_d_, CchAszToWsz(PszcOfStr(strFile), wsz, Length(wsz)) < Length(wsz));
	
	// Use GDI+ to read the image.  if it fails, use the shell.
	P<Bitmap> pbitmap (new Bitmap(wsz, false));

	CR crT;
	if (Gdiplus::Ok != pbitmap->GetPixel(0, 0, &crT))
		{
		// even better extractor: the shell get file info!			
		SHFILEINFO sfi;
		if (SHGetFileInfo(PszcOfStr(strFile), 0, &sfi, sizeof (sfi), SHGFI_ICON | SHGFI_LARGEICON) == 0)
			{
			// TODO: some way of conveying?
			}
		else
			{
			pbitmap.reset(Bitmap::FromHICON(sfi.hIcon));
			WindowsVerify(_d_, DestroyIcon(sfi.hIcon));
			}
		}
	else
		{
		if (pbitmap->GetWidth() > dxpDimension || pbitmap->GetHeight() > dypDimension)
			{
			DXP dxpShrink = pbitmap->GetWidth() > dxpDimension ? dxpDimension : pbitmap->GetWidth();
			DYP dypShrink = pbitmap->GetHeight() > dypDimension ? dypDimension : pbitmap->GetHeight();
			
			P<Bitmap> pbitmapShrunk (new Bitmap (dxpShrink, dypShrink, PixelFormat32bppARGB) );
			Graphics graphics (pbitmapShrunk.get());
			GdiplusVerify(_d_, graphics.DrawImage(pbitmap.get(), 0, 0, dxpShrink, dypShrink));
			
			pbitmap = pbitmapShrunk;
			}
		}

	InitializeHelper(pbitmap.get(), player, xpCenter, ypCenter);
	}
	
void VIEW_DRAW::DO_DROPBITMAP::InitializeHelper(Bitmap* pbitmap, const LAYER* player, XP xpCenter, YP ypCenter)
	{
	_dxp = pbitmap->GetWidth();
	_dyp = pbitmap->GetHeight();

	_rgrgcr = new CR[_dxp * _dyp];
	
	for (YP yp = 0; yp < _dyp; yp++)
		for (XP xp = 0; xp < _dxp; xp++)
			{
			// TODO: support transparency carryover?
			Gdiplus::Status status = pbitmap->GetPixel(xp, yp, &_rgrgcr[yp + xp*_dyp]);
			BYTE bparam;
			WORD wparam;
			if (status != Gdiplus::Ok || (FTransparentCr(_rgrgcr[yp + xp*_dyp], &bparam, &wparam)))
				_rgrgcr[yp + xp*_dyp] = crEmpty;
			}
	
	_xp = xpCenter - _dxp / 2;
	_yp = ypCenter - _dyp / 2;
	_pplane.reset (new PLANE (_player, this));
	}

F VIEW_DRAW::DO_DROPBITMAP::FCrGetXpYp(out CR* pcr, XP xp, YP yp) const
	{
	if (xp < _xp || xp >= _xp + _dxp)
		return fFalse;
	if (yp < _yp || yp >= _yp + _dyp)
		return fFalse;
	if (FEmptyCr(_rgrgcr[yp - _yp + _dyp*(xp - _xp)]))
		return fFalse;
	*pcr = _rgrgcr[yp - _yp + _dyp*(xp - _xp)];
	return fTrue;
	}
ERR VIEW_DRAW::DO_DROPBITMAP::ErrDo()
	{
	DOC_DRAW* pdoc = this->PdocOfDo();
	
	if (_xp < 0 || _yp < 0 || _xp + _dxp > dxpDimension || _yp + _dyp > dypDimension)
		{
		// must repair _rgrgcr so that it does not exceed the boundaries.
		YP ypShift = 0;
		XP xpShift = 0;
		YP ypActual = _yp;
		XP xpActual = _xp;
		DXP dxpActual = _dxp;
		DYP dypActual = _dyp;
		if (xpActual < 0)
			{
			dxpActual += xpActual;
			xpShift = -xpActual;
			xpActual = 0;
			}
		if (ypActual < 0)
			{
			dypActual += ypActual;
			ypShift = -ypActual;
			ypActual = 0;
			}
		if (xpActual + dxpActual > dxpDimension)
			dxpActual = dxpDimension - xpActual;
		if (ypActual + dypActual > dypDimension)
			dypActual = dypDimension - ypActual;
			
		CR* prgrgcr = new CR[dxpActual * dypActual];
		for (YP yp = 0; yp < dypActual; yp++)
			for (XP xp = 0; xp < dxpActual; xp++)
				{
				prgrgcr[yp + xp*dypActual] = _rgrgcr[(ypShift + yp) + (xpShift + xp)*_dyp];
				}
			
		P<MOD<DOC_DRAW> > pmod (new DOC_DRAW::MOD_REPLACERECT(pdoc, pdoc->PlayerNonConst(_player), xpActual, ypActual, dxpActual, dypActual, prgrgcr));
		pdoc->ApplyMod(pmod);
		
		delete[] prgrgcr;
		}
	else
		{
		P<MOD<DOC_DRAW> > pmod (new DOC_DRAW::MOD_REPLACERECT(pdoc, pdoc->PlayerNonConst(_player), _xp, _yp, _dxp, _dyp, _rgrgcr));
		pdoc->ApplyMod(pmod);
		}

	return errOk;
	}

P<DO_LINE<DOC_DRAW, VIEW_DRAW> > VIEW_DRAW::HIT_LINE_SEL::PdoLineTo(HIT* phitEnd) const
	{
	Assert(_d_, FIsA(phitEnd, HIT_LINE_SEL));
	HIT_LINE_SEL* phit_line_sel = reinterpret_cast<HIT_LINE_SEL*>(phitEnd);
/* haven't bothered with a DO yet 
	P<DO_LINE_SEL> pdoRet (new DO_LINE_SEL(_xpStart, _ypStart, phit_line_sel->_xpStart, phit_line_sel->_ypStart));
	return pdoRet;*/
	NotReached(_d_);
	P<DO_LINE<DOC_DRAW, VIEW_DRAW> > pdoNull (null);
	return pdoNull;
	}

VIEW_DRAW::HIT_LINE_SEL::~HIT_LINE_SEL()
	{
	DestroyCursor(_hcursor);
	}

	
// PRESS HIT
VIEW_DRAW::HIT_PRESS_LAYER::HIT_PRESS_LAYER (const LAYER* player) :
	HIT_PRESS<DOC_DRAW, VIEW_DRAW> (),
	_player (player)
	{
	_hcursor = LoadCursor(null, IDC_ARROW);
	}

bool VIEW_DRAW::HIT_PRESS_LAYER::operator==(const HIT& rhs) const
	{
	const HIT_PRESS_LAYER* phit_press_itNew = reinterpret_cast<const HIT_PRESS_LAYER*>(&rhs);
	if ((_player == phit_press_itNew->_player))
		return true;
	return false;
	}

HCURSOR VIEW_DRAW::HIT_PRESS_LAYER::Hcursor() const
	{
	return _hcursor;
	}


ERR VIEW_DRAW::DO_PRESS_LAYER::ErrDo()
	{
	VIEW_DRAW* pview = this->PviewOfDo();
	pview->SetLayer(_player);
	return errOk;
	}
	
P<DO_PRESS<DOC_DRAW, VIEW_DRAW> > VIEW_DRAW::HIT_PRESS_LAYER::PdoForPress() const
	{
	P<DO_PRESS_LAYER> pdo (new DO_PRESS_LAYER(_player));
	return pdo;
	}

VIEW_DRAW::HIT_PRESS_LAYER::~HIT_PRESS_LAYER()
	{
	DestroyCursor(_hcursor);
	}


// PRESS HIT
VIEW_DRAW::HIT_PRESS_CONTEXT::HIT_PRESS_CONTEXT (ICONTEXT icontext) :
	HIT_PRESS<DOC_DRAW, VIEW_DRAW> (),
	_icontext (icontext)
	{
	_hcursor = LoadCursor(null, IDC_ARROW);
	}

bool VIEW_DRAW::HIT_PRESS_CONTEXT::operator==(const HIT& rhs) const
	{
	const HIT_PRESS_CONTEXT* phit_press_itNew = reinterpret_cast<const HIT_PRESS_CONTEXT*>(&rhs);
	if ((_icontext == phit_press_itNew->_icontext))
		return true;
	return false;
	}

HCURSOR VIEW_DRAW::HIT_PRESS_CONTEXT::Hcursor() const
	{
	return _hcursor;
	}

ERR VIEW_DRAW::DO_PRESS_CONTEXT::ErrDo()
	{
	VIEW_DRAW* pview = this->PviewOfDo();
	pview->SetContext(_icontext);
	return errOk;
	}
	
P<DO_PRESS<DOC_DRAW, VIEW_DRAW> > VIEW_DRAW::HIT_PRESS_CONTEXT::PdoForPress() const
	{
	P<DO_PRESS_CONTEXT> pdo (new DO_PRESS_CONTEXT(_icontext));
	return pdo;
	}

VIEW_DRAW::HIT_PRESS_CONTEXT::~HIT_PRESS_CONTEXT()
	{
	DestroyCursor(_hcursor);
	}


// edit view

VIEW_DRAW::VIEW_DRAW(out std::list<VCMDDESC>* plistvcmddesc) :
	VIEW_GDI<DOC_DRAW>(plistvcmddesc),
	_icontext (0),
	_crPaint (crBlack),
	 _prgbitmapBackground (4),
	 _prgstrBackground (4),
	 _prgplane (26),
	 _kdraw (kdrawFreehand),
	 _fImpressionist (fTrue), // TODO: make it an option
	 _fAlphaBlendTopLayers (fTrue) // TODO: make it an option
	{
	const DOC_DRAW* pdoc = this->PdocOfView(); // should be const; can we limit constructor access to doc?

	scope
		{
		P<Bitmap> pbitmap (new Bitmap(VIEW_DRAW::Hinst(), reinterpret_cast<const WCHAR*>(MAKEINTRESOURCE(residBgWhite	))));
		_prgbitmapBackground.push_back(pbitmap);
		P<STR> pstr (new STR("white solid"));
		_prgstrBackground.push_back(pstr);
		}
	scope
		{
		P<Bitmap> pbitmap (new Bitmap(VIEW_DRAW::Hinst(), reinterpret_cast<const WCHAR*>(MAKEINTRESOURCE(residBgBlack))));
		_prgbitmapBackground.push_back(pbitmap);
		P<STR> pstr (new STR("black solid"));
		_prgstrBackground.push_back(pstr);
		}	
	scope
		{
		P<Bitmap> pbitmap (new Bitmap(VIEW_DRAW::Hinst(), reinterpret_cast<const WCHAR*>(MAKEINTRESOURCE(residBgRed))));
		_prgbitmapBackground.push_back(pbitmap);
		P<STR> pstr (new STR("rhododendron"));
		_prgstrBackground.push_back(pstr);
		}
	scope
		{
		P<Bitmap> pbitmap (new Bitmap(VIEW_DRAW::Hinst(), reinterpret_cast<const WCHAR*>(MAKEINTRESOURCE(residBgBlue))));
		_prgbitmapBackground.push_back(pbitmap);
		P<STR> pstr (new STR("sail away"));
		_prgstrBackground.push_back(pstr);
		}
	
	// go ahead and pay the cost for all the cached layers in the constructor...
	// while we have that progress dialog up
	for (ILAYER ilayer = 0; ilayer < pdoc->Clayer(); ilayer++)
		{
		P<PLANE> pplane (new PLANE(pdoc->PlayerForLayer(ilayer), null));
		_prgplane.push_back(pplane);
		}
		
	// start off viewing layer 0
	_player = pdoc->PlayerForLayer(0);
	
	_rgcrPalette[0] = crBlack;
	_rgcrPalette[1] = crGray;
	_rgcrPalette[2] = crLtGray;
	_rgcrPalette[3] = crWhite;
	_rgcrPalette[4] = crRed;
	_rgcrPalette[5] = crLtRed;
	_rgcrPalette[6] = crBlue;
	_rgcrPalette[7] = crLtBlue;
	_rgcrPalette[8] = crGreen;
	_rgcrPalette[9] = crLtGreen;
	_rgcrPalette[10] = crBrown;
	_rgcrPalette[11] = crYellow;
	_rgcrPalette[12] = crPurple;
	_rgcrPalette[13] = crGray;
	_rgcrPalette[14] = crWhite;
	_rgcrPalette[15] = crBlack;
	}
	
	
// DRAW HIT
VIEW_DRAW::HIT_DRAW_DRAW::HIT_DRAW_DRAW (const LAYER* player, XP xp, YP yp, CR cr) :
	HIT_DRAW<DOC_DRAW, VIEW_DRAW> (),
	_cr (cr),
	_player (player),
	_xp (xp),
	_yp (yp)
	{
	_hcursor = LoadCursor(VIEW_DRAW::Hinst(), MAKEINTRESOURCE(residCurDraw));
	}

bool VIEW_DRAW::HIT_DRAW_DRAW::operator==(const HIT& rhs) const
	{
	const HIT_DRAW_DRAW* phit_draw_drawNew = reinterpret_cast<const HIT_DRAW_DRAW*>(&rhs);
	if (_xp != phit_draw_drawNew->_xp)
		return false;
	if (_yp != phit_draw_drawNew->_yp)
		return false;
	if (_cr.ToCOLORREF() != phit_draw_drawNew->_cr.ToCOLORREF())
		return false;
	return true;
	}

HCURSOR VIEW_DRAW::HIT_DRAW_DRAW::Hcursor() const
	{
	return _hcursor;
	}

void VIEW_DRAW::DO_DRAW_DRAW::DrawNext(PT ptNext)
	{
	VIEW_DRAW* pview = this->PviewOfDo();
	DOC_DRAW* pdoc = this->PdocOfDo();
	
	RC rcWorkspace = pview->RcWorkspace();
	if (!rcWorkspace.FPtInRc(ptNext))
		return;
	
	PT ptInRc = ptNext - rcWorkspace.PtTopLeft();
	XP xp = ptInRc.Xp() / pview->SizCell().Dxp();
	YP yp = ptInRc.Yp() / pview->SizCell().Dyp();
	
	if ((xp == _xp) && (yp == _yp))
		return;

	UINT cpixelmods = 0;

	DXP dxp = xp - _xp;
	DYP dyp = yp - _yp;
	DXP cxp = dxp < 0 ? -dxp : dxp;
	DYP cyp = dyp < 0 ? -dyp : dyp;
	
	if (cxp < cyp)
		// draw along the yp axis since it will have more pixel points
		{
		DYP dypStep = _yp < yp ? 1 : -1;
		
		float fltDxpOverDyp = static_cast<float>(dxp) / static_cast<float>(dyp);
		float fltXp = static_cast<float>(_xp);
		YP ypT = _yp;
		Forever()
			{
			DXP xpSet = static_cast<XP>(fltXp);
			CR crBefore;
			if (_player->FCrGetXpYp(&crBefore, xpSet, ypT) && (crBefore.ToCOLORREF() == _cr.ToCOLORREF()))
				Noop();
			else
				{
				_rgrgfMask[xpSet][ypT] = fTrue;
				if (xpSet < _xpMin)
					_xpMin = xpSet;
				if (ypT < _ypMin)
					_ypMin = ypT;
				if (xpSet > _xpMax)
					_xpMax = xpSet;
				if (ypT > _ypMax)
					_ypMax = ypT;
				cpixelmods++;
				}
			
			fltXp += fltDxpOverDyp * dypStep;
			if (ypT == yp)
				break;
			ypT += dypStep;
			}					
		}
	else
		// draw on xp axis since it has equal or more points as yp axis.
		{
		DXP dxpStep = _xp < xp ? 1 : -1;
		
		float fltDypOverDxp = static_cast<float>(dyp) / static_cast<float>(dxp);
		float fltYp = static_cast<float>(_yp);
		XP xpT = _xp;
		Forever()
			{
			YP ypSet = static_cast<YP>(fltYp);
			CR crBefore;
			if (_player->FCrGetXpYp(&crBefore, xpT, ypSet) && (crBefore.ToCOLORREF() == _cr.ToCOLORREF()))
				Noop();
			else
				{
				_rgrgfMask[xpT][ypSet] = fTrue;
				if (xpT < _xpMin)
					_xpMin = xpT;
				if (ypSet < _ypMin)
					_ypMin = ypSet;
				if (xpT > _xpMax)
					_xpMax = xpT;
				if (ypSet > _ypMax)
					_ypMax = ypSet;
				cpixelmods++;
				}
				
			fltYp += fltDypOverDxp * dxpStep;
			if (xpT == xp)
				break;
			xpT += dxpStep;
			}
		}
	
	_xp = xp;
	_yp = yp;
	
	if (cpixelmods > 0)
		_pplane->Reset ();
	}

ERR VIEW_DRAW::DO_DRAW_DRAW::ErrDo()
	{
	DOC_DRAW* pdoc = this->PdocOfDo();

	// should there be an operation which puts a packed mask in?
	DXP dxpMask = (_xpMax - _xpMin + 1);
	DYP dypMask = (_ypMax - _ypMin + 1);
	CR* prgrgcr = new CR[dxpMask * dypMask];
	
	for (XP xp = 0; xp < dxpMask; xp++)
		for (YP yp = 0; yp < dypMask; yp++)
			{
			prgrgcr[yp + xp * dypMask] =  _rgrgfMask[xp + _xpMin][yp + _ypMin] ? _cr : crEmpty;
			}
	
	P<MOD<DOC_DRAW> > pmodReplaceBits (new DOC_DRAW::MOD_REPLACERECT (pdoc, pdoc->PlayerNonConst(_player), _xpMin, _ypMin, dxpMask, dypMask, prgrgcr));
	pdoc->ApplyMod(pmodReplaceBits);
	
	delete[] prgrgcr;
	return errOk;
	}

P<DO_DRAW<DOC_DRAW, VIEW_DRAW> > VIEW_DRAW::HIT_DRAW_DRAW::PdoForDraw() const
	{
	Review() // this makes it practically unusable
		{
		CR crBefore;
		if (_player->FCrGetXpYp(&crBefore, _xp, _yp) && crBefore.GetValue() == _cr.GetValue())
			{
			P<DO_DRAW_DRAW> pdo (null);
			return pdo;
			}
		}
			
	scope
		{
		P<DO_DRAW_DRAW> pdoRet (new DO_DRAW_DRAW(_player, _xp, _yp, _cr));
		return pdoRet;
		}
	}
	
P<DO> VIEW_DRAW::HIT_DRAW_DRAW::PdoForDrop(IDataObject* pdataobject) const
	{
	// if they drag and drop an object generate a DO with the appropriate bit matrix
	// we reuse the DO_DRAW_DRAW object for now as a test.
	scope
		{
		FORMATETC formatetc;
		formatetc.cfFormat = CF_HDROP; // CLIPFORMAT, predefined: list of filenames
		formatetc.ptd = null; // DVTARGETDEVICE, no target device
		formatetc.dwAspect = DVASPECT_CONTENT;  // DWORD, give me the content o
		formatetc.lindex = -1; // LONG, all of the data
		formatetc.tymed = TYMED_HGLOBAL; // DWORD, pass in global memory

		STGMEDIUM stgmedium;
		if (SUCCEEDED(pdataobject->GetData(&formatetc, &stgmedium)))
			{
			HDROP hdrop = reinterpret_cast<HDROP>(stgmedium.hGlobal);
			char* pglobal = reinterpret_cast<char*>(GlobalLock(stgmedium.hGlobal));
			CB cbGlobal = GlobalSize(stgmedium.hGlobal);
			GlobalUnlock(stgmedium.hGlobal);
			
			TCHAR szFileName[_MAX_PATH];
			::DragQueryFile(hdrop, 0, szFileName, _MAX_PATH);
			// implicitly frees stgmedium...
			::DragFinish(hdrop);

			P<DO> pdoRet (new DO_DROPBITMAP(szFileName, _player, _xp, _yp));
			return pdoRet;
			}
		}

	scope
		{
		FORMATETC formatetc;
		formatetc.cfFormat = CF_HDROP; // CLIPFORMAT, predefined: list of filenames
		formatetc.ptd = null; // DVTARGETDEVICE, no target device
		formatetc.dwAspect = DVASPECT_ICON;  // DWORD, give me the content o
		formatetc.lindex = -1; // LONG, all of the data
		formatetc.tymed = TYMED_GDI; // DWORD, pass in global memory

		STGMEDIUM stgmedium;
		if (SUCCEEDED(pdataobject->GetData(&formatetc, &stgmedium)))
			{
			HBITMAP hbitmap = reinterpret_cast<HBITMAP>(stgmedium.hGlobal);
			/*
			char* pglobal = reinterpret_cast<char*>(GlobalLock(stgmedium.hGlobal));
			CB cbGlobal = GlobalSize(stgmedium.hGlobal);
			GlobalUnlock(stgmedium.hGlobal);
			*/
			
			P<DO> pdoRet (new DO_DROPBITMAP(hbitmap, _player, _xp, _yp));
			ReleaseStgMedium(&stgmedium);
			return pdoRet;
			}
		}
		
	P<DO> pdoNull (null);
	return pdoNull;
	}

VIEW_DRAW::HIT_DRAW_DRAW::~HIT_DRAW_DRAW()
	{
	DestroyCursor(_hcursor);
	}


// PALETTE HIT
VIEW_DRAW::HIT_PRESS_PALETTE::HIT_PRESS_PALETTE (CR cr) :
	HIT_PRESS<DOC_DRAW, VIEW_DRAW> (),
	_cr (cr)
	{
	_hcursor = LoadCursor(VIEW_DRAW::Hinst(), MAKEINTRESOURCE(residCurEyedrop));
	}


bool VIEW_DRAW::HIT_PRESS_PALETTE::operator==(const HIT& rhs) const
	{
	const HIT_PRESS_PALETTE* phit_draw_drawNew = reinterpret_cast<const HIT_PRESS_PALETTE*>(&rhs);
	
	if (_cr.GetValue() != phit_draw_drawNew->_cr.GetValue())
		return false;
		
	return false;
	}

HCURSOR VIEW_DRAW::HIT_PRESS_PALETTE::Hcursor() const
	{
	return _hcursor;
	}


ERR VIEW_DRAW::DO_PRESS_PALETTE::ErrDo()
	{
	VIEW_DRAW* pview = this->PviewOfDo();
	DOC_DRAW* pdoc = this->PdocOfDo();

	pview->SetPaintColor(_cr);
	
	return errOk;
	}

P<DO_PRESS<DOC_DRAW, VIEW_DRAW> > VIEW_DRAW::HIT_PRESS_PALETTE::PdoForPress() const
	{
	P<DO_PRESS_PALETTE> pdoRet (new DO_PRESS_PALETTE(_cr));
	return pdoRet;
	}
	
VIEW_DRAW::HIT_PRESS_PALETTE::~HIT_PRESS_PALETTE()
	{
	DestroyCursor(_hcursor);
	}


// FILL HIT
VIEW_DRAW::HIT_PRESS_FILL::HIT_PRESS_FILL (const LAYER* player, XP xp, YP yp, CR cr) :
	HIT_PRESS<DOC_DRAW, VIEW_DRAW> (),
	_cr (cr),
	_player (player),
	_xpMin (xp),
	_xpMax (xp),
	_ypMin (yp),
	_ypMax (yp)
	{
	_hcursor = LoadCursor(VIEW_DRAW::Hinst(), MAKEINTRESOURCE(residCurFill));

	// initialize the mask.
	scope
		{
		for (XP xpT = 0; xpT < dxpDimension; xpT++)
			for (YP ypT = 0; ypT < dypDimension; ypT++)
				_rgrgfMask[xpT][ypT] = fFalse;
		}
	
	CR crBefore;
	if (_player->FCrGetXpYp(&crBefore, xp, yp) && (crBefore.GetValue() == cr.GetValue()))
		{
		// TODO: figure out how to convey a message to the UI saying why nothing will happen...?
		_xpMin = _ypMin = _xpMax = _ypMax = 0;
		return;
		}
		
	_rgrgfMask[xp][yp] = fTrue;
	// proceed out and flood fill the space...slow algorithm
	F fAnyChanged;
	do
		{
		fAnyChanged = fFalse;
		for (YP ypN = 0; ypN < dypDimension; ypN++)
			for (XP xpN = 0; xpN < dxpDimension; xpN++)
				if (!_rgrgfMask[xpN][ypN])
					{
					CR crCell;
					if (_player->FCrGetXpYp(&crCell, xpN, ypN) && crCell.GetValue() == crBefore.GetValue())
						{
						// if any adjacent mask bits are set, set this mask bit.
						for (YP ypT = ypN > 0 ? ypN - 1 : 0; ypT < (ypN + 2 < dypDimension ? ypN + 2 : dypDimension); ypT++)
							for (XP xpT = xpN > 0 ? xpN - 1 : 0; xpT < (xpN + 2 < dxpDimension ? xpN + 2 : dxpDimension); xpT++)
								if (_rgrgfMask[xpT][ypT])
									{
									_rgrgfMask[xpN][ypN] = fTrue;
									fAnyChanged = fTrue;
									if (xpN < _xpMin)
										_xpMin = xpN;
									if (xpN > _xpMax)
										_xpMax = xpN;
									if (ypN < _ypMin)
										_ypMin = ypN;
									if (ypN > _ypMax)
										_ypMax = ypN;
									}
								}
						}
		}
	while (fAnyChanged);
	}


bool VIEW_DRAW::HIT_PRESS_FILL::operator==(const HIT& rhs) const
	{
	const HIT_PRESS_FILL* phit_draw_drawNew = reinterpret_cast<const HIT_PRESS_FILL*>(&rhs);
	
	if (_cr.GetValue() != phit_draw_drawNew->_cr.GetValue())
		return false;
		
	// test for any overlaps...
	for (YP yp = 0; yp < dypDimension; yp++)
		for (XP xp = 0; xp < dxpDimension; xp++)
			{
			if (_rgrgfMask[xp][yp] && phit_draw_drawNew->_rgrgfMask[xp][yp])
				return true;
			}
			
	return false;
	}

HCURSOR VIEW_DRAW::HIT_PRESS_FILL::Hcursor() const
	{
	return _hcursor;
	}


ERR VIEW_DRAW::DO_PRESS_FILL::ErrDo()
	{
	DOC_DRAW* pdoc = this->PdocOfDo();

	// should there be an operation which puts a packed mask in?
	DXP dxpMask = (_xpMax - _xpMin + 1);
	DYP dypMask = (_ypMax - _ypMin + 1);
	CR* prgrgcr = new CR[dxpMask * dypMask];
	
	for (XP xp = 0; xp < dxpMask; xp++)
		for (YP yp = 0; yp < dypMask; yp++)
			{
			prgrgcr[yp + xp * dypMask] =  _rgrgfMask[xp + _xpMin][yp + _ypMin] ? _cr : crEmpty;
			}
	
	P<MOD<DOC_DRAW> > pmodReplaceBits (new DOC_DRAW::MOD_REPLACERECT (pdoc, pdoc->PlayerNonConst(_player), _xpMin, _ypMin, dxpMask, dypMask, prgrgcr));
	pdoc->ApplyMod(pmodReplaceBits);
	
	delete[] prgrgcr;
	return errOk;
	}

P<DO_PRESS<DOC_DRAW, VIEW_DRAW> > VIEW_DRAW::HIT_PRESS_FILL::PdoForPress() const
	{
	// TODO: better test
	if (_xpMin == 0)
		{
		// TODO: encode reason as an err?
		P<DO_PRESS_FILL> pdo (null /* new DO_NULL("Color is the same as selection color") */);
		return pdo;
		}
	else
		{
		P<DO_PRESS_FILL> pdoRet (new DO_PRESS_FILL(_player, _cr, _xpMin, _ypMin, _xpMax, _ypMax, _rgrgfMask));
		return pdoRet;
		}
	}
	
P<DO> VIEW_DRAW::HIT_PRESS_FILL::PdoForDrop(IDataObject* pdataobject) const
	{
	// if they drag and drop an object generate a DO with the appropriate bit matrix
	// we reuse the DO_PRESS_FILL object for now as a test.
	FORMATETC formatetc;
	formatetc.cfFormat = CF_HDROP; // CLIPFORMAT, predefined: list of filenames
	formatetc.ptd = null; // DVTARGETDEVICE, no target device
	formatetc.dwAspect = DVASPECT_CONTENT;  // DWORD, give me the content o
	formatetc.lindex = -1; // LONG, all of the data
	formatetc.tymed = TYMED_HGLOBAL; // DWORD, pass in global memory

	STGMEDIUM stgmedium;
	HRESULT hresult = pdataobject->GetData(&formatetc, &stgmedium);
	Assert(_d_, hresult == S_OK);
	
	if (SUCCEEDED(hresult))
		{
		HDROP hdrop = reinterpret_cast<HDROP>(stgmedium.hGlobal);
		char* pglobal = reinterpret_cast<char*>(GlobalLock(stgmedium.hGlobal));
		CB cbGlobal = GlobalSize(stgmedium.hGlobal);
		GlobalUnlock(stgmedium.hGlobal);
		
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hdrop, 0, szFileName, _MAX_PATH);
		::DragFinish(hdrop);

		// TODO: decide on how to handle equivalent fills when drag and drop might not be equivalent?
		/*
			{
			P<DO> pdoRet (new DO_DROPBITMAP(szFileName, _player, _xp, _yp));
			return pdoRet;
			}
		*/

/*		STR strFile = STR(pglobal, cbGlobal); */
/*		ReleaseStgMedium(&stgmedium);*/
		}

	P<DO> pdoNull (null);
	return pdoNull;
	}

VIEW_DRAW::HIT_PRESS_FILL::~HIT_PRESS_FILL()
	{
	DestroyCursor(_hcursor);
	}


// Idle processing

IPR VIEW_DRAW::IprIdle()
	{
	const DOC_DRAW* pdoc = this->PdocOfView();
	
	IPR ipr = iprComplete;
	for (ILAYER ilayer = 0; ilayer < pdoc->Clayer(); ilayer++)
		{
		if (FPollStop())
			return iprIncomplete;
		if (_prgplane[ilayer]->IprIdle() == iprIncomplete)
			ipr = iprIncomplete;
		}
	return ipr;
	}

KCELL VIEW_DRAW::KcellForXpYp(out CR* pcrRendered, out CR* pcrLiteral, const LAYER* player, XP xp, YP yp, const DO* pdo) const
	{
	const DOC_DRAW* pdoc = this->PdocOfView();
	
	if (pdo != null && FIsA(pdo, DO_DRAW_DRAW))
		{
		const DO_DRAW_DRAW* pdo_draw_draw = reinterpret_cast<const DO_DRAW_DRAW*>(pdo);
		if ((pdo_draw_draw != null) && (pdo_draw_draw->_player == player))
			return pdo_draw_draw->_pplane->KcellForXpYp(pcrRendered, pcrLiteral, xp, yp);
		}

	if (pdo != null && FIsA(pdo, DO_PRESS_FILL))
		{
		const DO_PRESS_FILL* pdo_draw_fill = reinterpret_cast<const DO_PRESS_FILL*>(pdo);
		if ((pdo_draw_fill != null) && (pdo_draw_fill->_player == player))
			return pdo_draw_fill->_pplane->KcellForXpYp(pcrRendered, pcrLiteral, xp, yp);
		}

	if (pdo != null && FIsA(pdo, DO_DROPBITMAP))
		{
		const DO_DROPBITMAP* pdo_dropbitmap = reinterpret_cast<const DO_DROPBITMAP*>(pdo);
		if (pdo_dropbitmap->_player == player)
			return pdo_dropbitmap->_pplane->KcellForXpYp(pcrRendered, pcrLiteral, xp, yp);
		}

	return _prgplane[pdoc->IlayerForPlayer(player)]->KcellForXpYp(pcrRendered, pcrLiteral, xp, yp);
	}

Bitmap* VIEW_DRAW::PbitmapOfLayer(const LAYER* player, const DO* pdo) const
	{
	const DOC_DRAW* pdoc = this->PdocOfView();
	
	if (pdo != null && FIsA(pdo, DO_DRAW_DRAW))
		{
		const DO_DRAW_DRAW* pdo_draw_draw = reinterpret_cast<const DO_DRAW_DRAW*>(pdo);
		if ((pdo_draw_draw != null) && (pdo_draw_draw->_player == player))
			return pdo_draw_draw->_pplane->Pbitmap();
		}

	if (pdo != null && FIsA(pdo, DO_PRESS_FILL))
		{
		const DO_PRESS_FILL* pdo_draw_fill = reinterpret_cast<const DO_PRESS_FILL*>(pdo);
		if ((pdo_draw_fill != null) && (pdo_draw_fill->_player == player))
			return pdo_draw_fill->_pplane->Pbitmap();
		}

	if (pdo != null && FIsA(pdo, DO_DROPBITMAP))
		{
		const DO_DROPBITMAP* pdo_dropbitmap = reinterpret_cast<const DO_DROPBITMAP*>(pdo);
		if (pdo_dropbitmap->_player == player)
			return pdo_dropbitmap->_pplane->Pbitmap();
		}
		
	return _prgplane[pdoc->IlayerForPlayer(player)]->Pbitmap();	
	}

P<Bitmap> VIEW_DRAW::PbitmapBottomLayers(ILAYER ilayerTop, const DO* pdo) const
	{
	const DOC_DRAW* pdoc = this->PdocOfView();
	P<Bitmap> pbitmap (new Bitmap (pdoc->Dxp(), pdoc->Dyp(), PixelFormat32bppARGB));
	// assume it starts out at transparent?
	Graphics graphics (pbitmap.get());

	for (ILAYER ilayerTest = 0; ilayerTest <= ilayerTop; ilayerTest++)
		{
		graphics.DrawImage(PbitmapOfLayer(pdoc->PlayerForLayer(ilayerTest), pdo), 0, 0, pdoc->Dxp(), pdoc->Dyp());
		}
	
	return pbitmap;
	}

P<Bitmap> VIEW_DRAW::PbitmapTopLayers(ILAYER ilayerTop) const
	{
	const DOC_DRAW* pdoc = this->PdocOfView();
	P<Bitmap> pbitmap (new Bitmap (pdoc->Dxp(), pdoc->Dyp(), PixelFormat32bppARGB));

	// assume it starts out at transparent?
	Graphics graphics (pbitmap.get());
	
	for (ILAYER ilayerTest = ilayerTop+1; ilayerTest < pdoc->Clayer(); ilayerTest++)
		{
		graphics.DrawImage(_prgplane[ilayerTest]->Pbitmap(), 0, 0, pdoc->Dxp(), pdoc->Dyp());
		}
	return pbitmap;
	}

void VIEW_DRAW::OnSize(SIZ siz)
	{
	_siz = siz;
	_sizCell = SIZ((siz.Dyp() - 16) / dypDimension, (siz.Dyp()-16) / dypDimension);
	}

// Paint routine
// Only piece of code which has access to the graphics device.  all other code paths must "influence" paint.
void VIEW_DRAW::Render(HDC hdc, const DO* pdo, const HIT* phitHover) const
	{
	DrawGraphics graphics (hdc);
	
	RC rcView = RC (PT (0, 0), _siz);
	const DOC_DRAW* pdoc = this->PdocOfView();
	
	// lay down a black background.
	scope
		{
		SolidBrush brushBlack (crBlack);
		graphics.FillRectangle (&brushBlack, RectF(rcView.XpLeft(), rcView.YpTop(), rcView.DxpWidth(), rcView.DypHeight()));
		}
	
	const DO_DRAW_DRAW* pdo_draw_draw = null;
	if (pdo != null && FIsA(pdo, DO_DRAW_DRAW))
		{
		pdo_draw_draw = reinterpret_cast<const DO_DRAW_DRAW*>(pdo);
		}
	const DO_DROPBITMAP* pdo_dropbitmap = null;
	if (pdo != null && FIsA(pdo, DO_DROPBITMAP))
		{
		pdo_dropbitmap = reinterpret_cast<const DO_DROPBITMAP*>(pdo);
		}
		
	const HIT_DRAW_DRAW* phit_draw_draw = null;
	if (phitHover != null && FIsA(phitHover, VIEW_DRAW::HIT_DRAW_DRAW))
		{
		phit_draw_draw = reinterpret_cast<const HIT_DRAW_DRAW*>(phitHover);
		}
	const HIT_PRESS_FILL* phit_draw_fill = null;
	if (phitHover != null && FIsA(phitHover, VIEW_DRAW::HIT_PRESS_FILL))
		{
		phit_draw_fill = reinterpret_cast<const HIT_PRESS_FILL*>(phitHover);
		}	
	const HIT_PRESS_LAYER* phit_press_layer = null;
	if (phitHover != null && FIsA(phitHover, VIEW_DRAW::HIT_PRESS_LAYER))
		{
		phit_press_layer = reinterpret_cast<const HIT_PRESS_LAYER*>(phitHover);
		}
	
	const LAYER* playerActual = _player;
	const DO_PRESS_LAYER* pdo_press_layer = null;
	if (pdo != null && FIsA(pdo, DO_PRESS_LAYER))
		{
		pdo_press_layer = reinterpret_cast<const DO_PRESS_LAYER*>(pdo);
		playerActual = pdo_press_layer->_player;
		}

	const HIT_PRESS_CONTEXT* phit_press_context = null;
	if (phitHover != null && FIsA(phitHover, VIEW_DRAW::HIT_PRESS_CONTEXT))
		{
		phit_press_context = reinterpret_cast<const HIT_PRESS_CONTEXT*>(phitHover);
		}

	ICONTEXT icontextActual = _icontext;
	const DO_PRESS_CONTEXT* pdo_press_context = null;
	if (pdo != null && FIsA(pdo, DO_PRESS_CONTEXT))
		{
		pdo_press_context = reinterpret_cast<const DO_PRESS_CONTEXT*>(pdo);
		icontextActual = pdo_press_context->_icontext;
		}

	ILAYER ilayerActual = pdoc->IlayerForPlayer(playerActual);
		
	P<Bitmap> pbitmapBottom = PbitmapBottomLayers(pdoc->IlayerForPlayer(playerActual), pdo);
	P<Bitmap> pbitmapTop = PbitmapTopLayers(pdoc->IlayerForPlayer(playerActual));
	
	RC rcThumbnail = RcThumbnail();
	// set down the thumbnail on the current "context"
	scope
		{
		graphics.DrawImage(_prgbitmapBackground[icontextActual], rcThumbnail.XpLeft(), rcThumbnail.YpTop(), _prgbitmapBackground[icontextActual]->GetWidth(), _prgbitmapBackground[icontextActual]->GetHeight());
		
		graphics.DrawImage(pbitmapBottom.get(), rcThumbnail.XpLeft() + 82, rcThumbnail.YpTop() + 49, pdoc->Dxp(), pdoc->Dyp());
		graphics.DrawImage(pbitmapTop.get(), rcThumbnail.XpLeft() + 82, rcThumbnail.YpTop() + 49, pdoc->Dxp(), pdoc->Dyp());
		}
	
	// show a list of available "contexts" so user can preview the icon in various settings
	RC rcContext = RcContext();
	scope
		{
		YP ypTab = 0;
		for (ICONTEXT icontext = 0; icontext < this->Ccontext(); icontext++)
			{
			F fCurrent = icontext == icontextActual;
			F fPressed = pdo_press_context != null && icontext == pdo_press_context->_icontext; 
			F fHover = phit_press_context != null && icontext == phit_press_context->_icontext;
			graphics.DrawTab(this->StrContext(icontext), RC(PT (rcContext.XpLeft(), rcContext.YpTop() + ypTab), SIZ(DrawGraphics::dxpTab, DrawGraphics::dypTab)), fCurrent, fPressed, fHover);
			
			ypTab += DrawGraphics::dypTab;
			}
		}

	RC rcWorkspace = RcWorkspace();
	scope
		{
		PixelOffsetMode pixeloffsetmode = graphics.GetPixelOffsetMode();
		GdiplusVerify(_d_, graphics.SetPixelOffsetMode(PixelOffsetModeHalf));
		
		// first lay down some sort of basis layer
		HatchBrush brushBasis (HatchStyleForwardDiagonal, CR (160, 160, 160), CR (224, 224, 224));
		graphics.FillRectangle(&brushBasis, rcWorkspace.XpLeft(), rcWorkspace.YpTop(), pdoc->Dxp() * _sizCell.Dxp(), pdoc->Dyp() * _sizCell.Dyp());

		const float fltAlphaPct = 0.3f;
		if (_fImpressionist)
			{
			// Use GDI+ to lay down the underneath layers with proper diffusion gradients so they don't look too blocky
			graphics.DrawImage(pbitmapBottom.get(), rcWorkspace.XpLeft(), rcWorkspace.YpTop(), pdoc->Dxp() * _sizCell.Dxp(), pdoc->Dyp() * _sizCell.Dyp());
			
			if (_fAlphaBlendTopLayers && (ilayerActual != pdoc->Clayer() - 1))
				{
				// REVIEW: should we blend the bitmaps together before blitting, or leave the layers apart to do clouding?
				
				// Initialize the color matrix.
				// Note the value 0.8 in row 4, column 4.
				ColorMatrix colorMatrix =
					{ 
					1, 0, 0, 0, 0,
					0, 1, 0, 0, 0,
					0, 0, 1, 0, 0,
					0, 0, 0, fltAlphaPct, 0, 
					0, 0, 0, 0, 1
					};
					
				// Create an ImageAttributes object and set its color matrix.
				ImageAttributes imageAtt;
				imageAtt.SetColorMatrix(&colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

				// now layer on top an alpha
				RectF rectDest (rcWorkspace.XpLeft(), rcWorkspace.YpTop(), pdoc->Dxp() * _sizCell.Dxp(), pdoc->Dyp() * _sizCell.Dyp());
				graphics.DrawImage(pbitmapTop.get(), rectDest, 0, 0, pbitmapTop.get()->GetWidth(), pbitmapTop.get()->GetHeight(), UnitPixel, &imageAtt);
				}
				
			GdiplusVerify(_d_, graphics.SetPixelOffsetMode(pixeloffsetmode));
			}
		else
			{
			for (XP xp = 0; xp < pdoc->Dxp(); xp++)
				for (YP yp = 0; yp < pdoc->Dyp(); yp++)
					{
					CR crBottom;
					GdiplusVerify(_d_, pbitmapBottom->GetPixel(xp, yp, &crBottom));
					if (!FTransparentCr(crBottom))
						{
						SolidBrush brushFill (crBottom);
						GdiplusVerify(_d_, graphics.FillRectangle(&brushFill, RectF(rcWorkspace.XpLeft() + xp*_sizCell.Dxp(), rcWorkspace.YpTop() + yp*_sizCell.Dyp(), _sizCell.Dxp(), _sizCell.Dyp())));	
						}
					
					if (_fAlphaBlendTopLayers)
						{
						CR crTop;
						GdiplusVerify(_d_, pbitmapTop->GetPixel(xp, yp, &crTop));
						if (!FTransparentCr(crTop))
							{
							SolidBrush brushFill (Color::MakeARGB(fltAlphaPct*crTop.GetA(), crTop.GetR(), crTop.GetG(), crTop.GetB()));
							GdiplusVerify(_d_, graphics.FillRectangle(&brushFill, RectF(rcWorkspace.XpLeft() + xp*_sizCell.Dxp(), rcWorkspace.YpTop() + yp*_sizCell.Dyp(), _sizCell.Dxp(), _sizCell.Dyp())));	
							}
						}
					}
			}
		}
		
	// layer on the hover and pixel cell effects, or just the cells if !_fImpressionist
	scope
		{
		Pen penOutline (crGray);
		SolidBrush brushUnknown (crWhite);
		Font myFont (L"System", _sizCell.Dyp() / 2, FontStyleRegular);
		SolidBrush brushText(crGray);
		
		for (XP xp = 0; xp < pdoc->Dxp(); xp++)
			for (YP yp = 0; yp < pdoc->Dyp(); yp++)
				{
				F fInspect = fFalse;
				
				if (phit_draw_fill != null)
					{
					fInspect = phit_draw_fill->_rgrgfMask[xp][yp];
					}
					
				if (phit_draw_draw != null || pdo_draw_draw != null)
					{
					// how far away are we from the central pixel?
					XP xpHover = phit_draw_draw != null ? phit_draw_draw->_xp : pdo_draw_draw->_xp;
					YP ypHover = phit_draw_draw != null ? phit_draw_draw->_yp : pdo_draw_draw->_yp;
					
					if ((xpHover >= xp && xpHover < xp + 3) || (xpHover <= xp && xpHover + 3 > xp))
						if ((ypHover >= yp && ypHover < yp + 3) || (ypHover <= yp && ypHover + 3 > yp))
							fInspect = fTrue;
					}
				
				F fCertain = fTrue;
				KCELL kcell;
				CR crRendered;
				CR crLiteral;
				ILAYER ilayerHit = ilayerActual + 1;
				do
					{
					ilayerHit--;
					kcell = KcellForXpYp(&crRendered, &crLiteral, pdoc->PlayerForLayer(ilayerHit), xp, yp, pdo);
					if (!FRenderCertainKcell(kcell))
						fCertain = fFalse;
					}
				while ((!FLiteralKcell(kcell) && !FRenderKcell(kcell)) && (ilayerHit > 0));

				F fLabeled = fFalse;
				if (fInspect)
					{
						/*CR crInDoc;
						if (pdoc->FCrGetXpYp(&crInDoc, ilayerHit, xp, yp))
							{
							graphics.DrawTriangle(crInDoc, PT(xpStart + xp*_sizCell.Dxp(), ypStart + yp*_sizCell.Dyp()), _sizCell.Dyp()/2);
							}*/
							
					if (FLiteralKcell(kcell))
						{
						SolidBrush brushFill (crLiteral);
						GdiplusVerify(_d_, graphics.FillRectangle(&brushFill, RectF(rcWorkspace.XpLeft() + xp*_sizCell.Dxp(), rcWorkspace.YpTop() + yp*_sizCell.Dyp(), _sizCell.Dxp() - 1, _sizCell.Dyp() -1)));				
						// label with a letter if the pixel is "not of this layer"
						if ((ilayerHit != ilayerActual))
							{
							// Create a string.
							WCHAR string[2];
							wcscpy(string, L"a");
							string[0] += ilayerHit;
							
							// Initialize arguments.
							PointF origin (rcWorkspace.XpLeft() + xp*_sizCell.Dxp() + 2, rcWorkspace.YpTop() + yp*_sizCell.Dyp() + 2);

							// Draw string.
							GdiplusVerify(_d_, graphics.DrawString(string, wcslen(string), &myFont, origin, &brushText));
							fLabeled = fTrue;
							}
						}
					else
						{
						// do something?
						// label with an X if the cell is an outline cell, not real...
						if (fCertain && FRenderKcell(kcell))
							GdiplusVerify(_d_, graphics.DrawLine(&penOutline, PointF(rcWorkspace.XpLeft() + xp*_sizCell.Dxp(), rcWorkspace.YpTop() + yp*_sizCell.Dyp()), PointF(rcWorkspace.XpLeft() + xp*_sizCell.Dxp() + _sizCell.Dxp() - 1, rcWorkspace.YpTop() + yp*_sizCell.Dyp() + _sizCell.Dyp() -1)));
						}
						
					GdiplusVerify(_d_, graphics.DrawRectangle(&penOutline, RectF(rcWorkspace.XpLeft() + xp*_sizCell.Dxp(), rcWorkspace.YpTop() + yp*_sizCell.Dyp(), _sizCell.Dxp() - 1, _sizCell.Dyp() -1)));
					}
									
				if (!fCertain && !fLabeled)
					{
					// draw the "unknown dot" in the center of the cell.
					SIZ sizDot = SIZ (_sizCell.Dxp() / 6, _sizCell.Dyp() / 6);
					if (sizDot.Dxp() != 0 && sizDot.Dyp() != 0)
						{
						GdiplusVerify(_d_, graphics.FillRectangle(&brushUnknown, rcWorkspace.XpLeft() + xp*_sizCell.Dxp() + _sizCell.Dxp()/2 - sizDot.Dxp()/2, rcWorkspace.YpTop() + yp*_sizCell.Dyp() + _sizCell.Dyp()/2 - sizDot.Dyp() / 2, sizDot.Dxp(), sizDot.Dyp()));
						GdiplusVerify(_d_, graphics.DrawRectangle(&penOutline, rcWorkspace.XpLeft() + xp*_sizCell.Dxp() + _sizCell.Dxp()/2 - sizDot.Dxp()/2 - 1, rcWorkspace.YpTop() + yp*_sizCell.Dyp() + _sizCell.Dyp()/2 - sizDot.Dyp()/2 - 1, sizDot.Dxp() + 1, sizDot.Dyp() + 1));
						}
					}
				}
			}
		
	RC rcLayer = RcLayer();
	// now draw the layer tabs
	scope
		{
		YP ypTab = 0;
		for (ILAYER ilayerTab = 0; ilayerTab < pdoc->Clayer(); ilayerTab++)
			{
			ILAYER ilayer = pdoc->Clayer() - ilayerTab - 1;
			const LAYER* player = pdoc->PlayerForLayer(ilayer);
			F fCurrent = ilayer == ilayerActual;
			F fPressed = pdo_press_layer != null && player == pdo_press_layer->_player; 
			F fHover = phit_press_layer != null && player == phit_press_layer->_player;
			
			CH pszLayer[] = "a";
			pszLayer[0] += ilayer;
			
			graphics.DrawTab(pszLayer, RC(PT (rcLayer.XpLeft(), rcLayer.YpTop() + ypTab), SIZ (DrawGraphics::dxpTab, DrawGraphics::dypTab)), fCurrent, fPressed, fHover);
			
			// use GDI+ to stretch a little mini version of each bitmap into the tab
			
			graphics.DrawImage(PbitmapOfLayer(pdoc->PlayerForLayer(ilayer), pdo), rcLayer.XpLeft() + DrawGraphics::dxpTab - DrawGraphics::dypTab, ypTab, DrawGraphics::dypTab, DrawGraphics::dypTab);
			
			ypTab += DrawGraphics::dypTab;
			}
		}
		
	// draw the palette
	scope
		{
		RC rcPalette = RcPalette();
		DXP dxpSwatch = (rcPalette.DxpWidth() / ccrPalette);
		for (I(CR) icr = 0; icr < ccrPalette; icr++)
			{
			SolidBrush brushSwatch (_rgcrPalette[icr]);
			graphics.FillRectangle(&brushSwatch, RectF(dxpSwatch*icr, rcPalette.YpTop(), dxpSwatch, rcPalette.DypHeight()));
			}
		}
		
	if (0)
		scope
			{
			// silly counter to help me realize how many redraws are happening so I can ponder that.
			static UINT crender = 0;
			crender++;
			
			CH szNumber[20];
			wsprintf(szNumber, "r: %d", crender);
			// Create a string.
			WCHAR wsz[20];
			Verify(_d_, CchAszToWsz(szNumber, wsz, Length(wsz)) < Length(wsz));

			// Initialize arguments.
			Font myFont (L"System", 8, FontStyleRegular);
			PointF origin (rcView.XpLeft(), rcView.YpBottom() - 20);
			SolidBrush brushText(crLtGray);

			// Draw string.
			graphics.DrawString(wsz, CchOfWsz(wsz), &myFont, PointF(0,0), &brushText);
			}
	}
// Hit methods say what would happen if a click were done.  There is no guarantee that
// the mouse has been clicked...in fact, it HAS NOT NECESSARILY.  But this needs to 
// determine what would happen, by returning the right kind of hit element.
RC VIEW_DRAW::RcThumbnail() const
	{
	return RC(PT (0, 0), SIZ(this->dxpBackground, this->dypBackground));
	}
RC VIEW_DRAW::RcContext() const
	{
	RC rcThumbnail = RcThumbnail();
	return RC (rcThumbnail.PtBottomLeft() + SIZ (0, _sizCell.Dyp()), SIZ (DrawGraphics::dxpTab, this->Ccontext()*DrawGraphics::dypTab));
	}
RC VIEW_DRAW::RcLayer() const
	{
	RC rcWorkspace = RcWorkspace();
	const DOC_DRAW* pdoc = this->PdocOfView();	
	return RC (rcWorkspace.PtTopRight() + SIZ (_sizCell.Dxp(), 0), SIZ (DrawGraphics::dxpTab, DrawGraphics::dypTab*pdoc->Clayer()));
	}
RC VIEW_DRAW::RcWorkspace() const
	{
	RC rcThumbnail = RcThumbnail();
	const DOC_DRAW* pdoc = this->PdocOfView();	
	return RC (rcThumbnail.PtTopRight() + SIZ (_sizCell.Dxp(), 0), SIZ (pdoc->Dxp() * _sizCell.Dxp(), pdoc->Dyp() * _sizCell.Dyp()));
	}
RC VIEW_DRAW::RcPalette() const
	{
	return RC (PT (0, _siz.Dyp() - 16), SIZ (_siz.Dxp(), 16));
	}
P<HIT> VIEW_DRAW::PhitFromPt(PT pt) const
	{
	const DOC_DRAW* pdoc = this->PdocOfView();	
	
	RC rcContext = RcContext();
	if (rcContext.FPtInRc(pt))
		{
		PT ptInRc = pt - rcContext.PtTopLeft();
		ICONTEXT icontext = ptInRc.Yp() / DrawGraphics::dypTab;
		Assert(_d_, icontext < this->Ccontext());
		
		P<HIT> phit (new HIT_PRESS_CONTEXT(icontext));
		return phit;
		}
		
	RC rcLayer = RcLayer();
	if (rcLayer.FPtInRc(pt))
		{
		PT ptInRc = pt - rcLayer.PtTopLeft();
		ILAYER ilayerTab = ptInRc.Yp() / DrawGraphics::dypTab;		
		Assert(_d_, ilayerTab < pdoc->Clayer());
		
		P<HIT> phit (new HIT_PRESS_LAYER(pdoc->PlayerForLayer(pdoc->Clayer() - ilayerTab - 1)));
		return phit;
		}
		
	RC rcPalette = RcPalette();
	if (rcPalette.FPtInRc(pt))
		{
		PT ptInRc = pt - rcPalette.PtTopLeft();
		
		P<HIT> phit (new HIT_PRESS_PALETTE (_rgcrPalette[ptInRc.Xp() / (rcPalette.DxpWidth() / ccrPalette)]));
		return phit;
		}
		
	RC rcWorkspace = RcWorkspace();
	if (rcWorkspace.FPtInRc(pt))
		{
		PT ptInRc = pt - rcWorkspace.PtTopLeft();		
		XP xp = ptInRc.Xp() / _sizCell.Dxp();
		YP yp = ptInRc.Yp() / _sizCell.Dyp();
		Assert(_d_, (xp < pdoc->Dxp()) && (yp < pdoc->Dyp()));
	
		P<HIT> phit;
		if (_kdraw == kdrawFreehand)
			phit.reset(new VIEW_DRAW::HIT_DRAW_DRAW(_player, xp, yp, _crPaint));
		else if (_kdraw == kdrawFill)
			phit.reset(new VIEW_DRAW::HIT_PRESS_FILL(_player, xp, yp, _crPaint));
		else
			NotReached(_d_);
		return phit;
		}
		
	scope
		{
		P<HIT> phit (null);
		return phit;
		}
	}

void VIEW_DRAW::BeforeDocApply(in const MOD<DOC_DRAW>* pmod)
	{
	if (FIsA(pmod, DOC_DRAW::MOD_REPLACERECT))
		{
		// ignore this, just take care of it in the after.
		// ideally we would free the bitmap here, perhaps?
		}
	else
		{
		}
	}
	
void VIEW_DRAW::AfterDocApply(in const MOD<DOC_DRAW>* pmod)
	{
	DOC_DRAW* pdoc = pmod->PdocOfMod();
	// update the view structures
	if (FIsA(pmod, DOC_DRAW::MOD_REPLACERECT))
		{
		// update the view cache for this layer, pay the cost in the command body
		const DOC_DRAW::MOD_REPLACERECT* pmod_replacerect = reinterpret_cast<const DOC_DRAW::MOD_REPLACERECT*>(pmod);
		_prgplane[pmod_replacerect->_ilayer]->Reset();
		}
	else if (FIsA(pmod, DOC_DRAW::MOD_SETPIXEL))
		{
		// update the view cache for this layer, pay the cost in the command body
		const DOC_DRAW::MOD_SETPIXEL* pmod_setpixel = reinterpret_cast<const DOC_DRAW::MOD_SETPIXEL*>(pmod);
		_prgplane[pmod_setpixel->_ilayer]->Reset();
		}
	else if (FIsA(pmod, DOC_DRAW::MOD_DELETELAYER))
		{
		// no more view caching for this layer!
		const DOC_DRAW::MOD_DELETELAYER* pmod_deletelayer = reinterpret_cast<const DOC_DRAW::MOD_DELETELAYER*>(pmod);
		_prgplane.erase(_prgplane.ToIter(pmod_deletelayer->_ilayer));
		
		// TODO: better invariant?
		_player = pdoc->PlayerForLayer(0);
		}
	else if (FIsA(pmod, DOC_DRAW::MOD_INSERTLAYER))
		{
		// start view cache for the new layer
		const DOC_DRAW::MOD_INSERTLAYER* pmod_insertlayer = reinterpret_cast<const DOC_DRAW::MOD_INSERTLAYER*>(pmod);
		P<PLANE> pplane (new PLANE(pdoc->PlayerForLayer(pmod_insertlayer->_ilayer), null));
		_prgplane.insert(pmod_insertlayer->_ilayer, pplane);
		} 
	else if (FIsA(pmod, DOC_DRAW::MOD_SETEFFECT))
		{
		// update the view cache for this layer, pay the cost in the command body
		const DOC_DRAW::MOD_SETEFFECT* pmod_seteffect = reinterpret_cast<const DOC_DRAW::MOD_SETEFFECT*>(pmod);
		_prgplane[pmod_seteffect->_ilayer]->Reset();
		}
	else
		NotReached(_d_);
	}
	
void VIEW_DRAW::BeforeDocUnapply(in const MOD<DOC_DRAW>* pmod)
	{
	// update the view structures
	if (FIsA(pmod, DOC_DRAW::MOD_REPLACERECT))
		{
		// ignore this, just take care of it in the after.
		// ideally we would free the bitmap here, perhaps?
		}
	else
		{
		}
	}

void VIEW_DRAW::AfterDocUnapply(in const MOD<DOC_DRAW>* pmod)
	{
	DOC_DRAW* pdoc = pmod->PdocOfMod();
	
	// update the view structures
	if (FIsA(pmod, DOC_DRAW::MOD_REPLACERECT))
		{
		// update the view cache for this layer, pay the cost in the command body
		const DOC_DRAW::MOD_REPLACERECT* pmod_replacerect = reinterpret_cast<const DOC_DRAW::MOD_REPLACERECT*>(pmod);
		_prgplane[pmod_replacerect->_ilayer]->Reset();
		}
	else if (FIsA(pmod, DOC_DRAW::MOD_SETPIXEL))
		{
		// update the view cache for this layer, pay the cost in the command body
		const DOC_DRAW::MOD_SETPIXEL* pmod_setpixel = reinterpret_cast<const DOC_DRAW::MOD_SETPIXEL*>(pmod);
		_prgplane[pmod_setpixel->_ilayer]->Reset();
		}
	else if (FIsA(pmod, DOC_DRAW::MOD_DELETELAYER))
		{
		// start view cache for the new layer
		const DOC_DRAW::MOD_DELETELAYER* pmod_deletelayer = reinterpret_cast<const DOC_DRAW::MOD_DELETELAYER*>(pmod);
		P<PLANE> pplane (new PLANE(pdoc->PlayerForLayer(pmod_deletelayer->_ilayer), null));
		_prgplane.insert(pmod_deletelayer->_ilayer, pplane);
		}
	else if (FIsA(pmod, DOC_DRAW::MOD_INSERTLAYER))
		{
		// no more view caching for this layer!
		const DOC_DRAW::MOD_INSERTLAYER* pmod_insertlayer = reinterpret_cast<const DOC_DRAW::MOD_INSERTLAYER*>(pmod);
		_prgplane.erase(_prgplane.ToIter(pmod_insertlayer->_ilayer));
		
		// TODO: better invariant?
		_player = pdoc->PlayerForLayer(0);
		}
	else if (FIsA(pmod, DOC_DRAW::MOD_SETEFFECT))
		{
		// update the view cache for this layer, pay the cost in the command body
		const DOC_DRAW::MOD_SETEFFECT* pmod_seteffect = reinterpret_cast<const DOC_DRAW::MOD_SETEFFECT*>(pmod);
		_prgplane[pmod_seteffect->_ilayer]->Reset();
		}
	else
		NotReached(_d_);
	}

VIEW_DRAW::~VIEW_DRAW()
	{
	}