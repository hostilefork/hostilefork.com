Rebol [
	Title: "Make Hostilefork Blog"
	Description: {
		This is the script for building the static template files behind
		the website hostilefork.com.  It requires that you have a subdirectory
		called "draem" containing the contents of the Draem repsitory:

			https://github.com/hostilefork/draem

		See the Draem documentation (and/or source) for config options.
	}

	Author: "Hostile Fork"
	Home: http://hostilefork.com
	License: 'bsd

	Date: 11-Jan-2014
	Version: 0.1

	; Header conventions: http://www.rebol.org/one-click-submission-help.r
	File: %make-hostilefork.reb
	Type: 'tool
	Level: 'intermediate

	Usage: {
		Requires Rebol 3.  From the command line, run:

			$ rebol make-hostilefork.reb

		Generated static pages will be found in the templates directory.
	}
]

do %draem/draem.reb

; Each Draem entry has an ability to do tweak and modify the configuration for
; that page with settings in the header block.  However, site-wide
; configuration is currently done by passing an object into Draem.  This
; feels a little ad-hoc, but systems like Django wind up doing similar things.

draem/set-config object compose [
	site-url: http://blog.hostilefork.com/
	site-name: "Hostilefork.com Blog"
	site-title: {HostileFork Blog}
	site-tagline: {Not actually *hostile* (just a bit irate.)}
	site-author: {HostileFork}

	rss-tag: {blog.hostilefork.com}

	site-prologue-html: {<div style="text-align: right; margin-right: 8px;"><p><img src="http://hostilefork.com/media/feed-icon-14x14.png" alt="Feed Icon" />&nbsp;<a href="http://blog.hostilefork.com/feed/">RSS 1.0 XML Feed</a> available </p></div>}

	site-intro: [
		print {An occasionally-updated collection of articles and notes.  Mostly about software development.  Popular topics are C++ and the [Rebol and Red](http://blog.hostilefork.com/why-rebol-red-parse-cool/) languages, along with general programming philosophy and puzzle-solving.  For project summaries, demos, and overview videos please see the portal at [http://hostilefork.com](http://hostilefork.com).}

		print {Articles on blog.hostilefork.com originated from a scrape and rewrite of content that was previously on the main page.  All entries were transformed from WordPress to a [REN](https://github.com/humanistic/REN) *("REadable Notation")* format, and run through a custom experimental static site builder called [Draem](http://draem.hostilefork.com).  This means the "source code" for entries are available on GitHub--feel free to send pull requests for any edits or corrections:}

		link https://github.com/hostilefork/hostilefork.com/tree/master

		print {The produced HTML is deliberately simple, with the links here enough for search engines (and me) to find the pages.  If you'd like, you can [browse the site by tag](http://hostilefork.com/tag/).  It should also support navigating the entries with the left and right arrow keys to make reading easier.}

		print {Ideally I would post more often, but in practice writing things on StackOverflow is easier.  So this just winds up as a repository for things that aren't a fit for that medium:}

		button [https://stackoverflow.com/users/flair/211160.png 208x58 "profile for HostileFork on Stack Exchange" http://stackoverflow.com/users/211160/hostilefork?tab=answers&sort=votes]

		heading {Master List of Entries}
	]

	site-footer: [

		button [http://hostilefork.com/media/fork-card-back-320x560.png 320x560 "Business Card from SXSW" http://hostilefork.com/hire-the-fork/]

	]

	site-trailer-html: {<div style="text-align: right;"><span>Copyright (c) 2007-2018 hostilefork.com</span></div>}

	; This is made long to work around the page width issue where we set a
	; maximum-width but no minimum-width.  In order to make sure we are
	; taking advantage of a natural device width a relatively long string
	; here that will break is a fairly natural fit.
	site-epilogue-html: {<div style="text-align: center;"><p>Project names and graphic designs are All Rights Reserved, unless otherwise noted.  Software codebases are governed by licenses included in their distributions.  Posts on <a href="http://blog.hostilefork.com">blog.hostilefork.com</a>
		are licensed under the Creative Commons <a href="http://creativecommons.org/licenses/by-nc-sa/4.0/">BY-NC-SA 4.0</a> license, and may be excerpted or adapted under the terms of that license for noncommercial purposes.</p></div>}

	google-analytics: [
		property: "hostilefork.com"
		tracking-id: "UA-2926272-2"
	]

	site-toplevel-slugs: [
		%homepage
		%projects
		%about
		%albumist
		%copylight
		%blackhighlighter
		%hire-the-fork
		%hoist
		%legal
		%nocycle
		%nstate
		%rebmu
		%rubol
		%thinker-qt
		%uscii
		%jquery-numband
		%flatworm
		%titlewait
		%draem
		%methyl
		%benzene
		%rencpp
	]

	entries-dir: (rejoin [system/options/path %entries/])
	templates-dir: (rejoin [system/options/path %templates/])

	template-path-from-header: function [header [object!]] [
		rejoin [templates-dir stringify/dashes entry/header/category "/" entry/header/slug %.html]
	]

	;-- NOTE: Somehow Google crawled my *unpublished* site and picked up
	;-- an unfinished URL scheme.  So now I have to redirect those URLs
	;-- as well.  Or decide I don't care if they break, as no one but
	;-- them link to it.

	;-- Required url-from-header hook
	url-from-header: function [header [object!]] [
		either find site-toplevel-slugs header/slug [
			;-- Should we go straight to the subdomains?
			rejoin [site-url header/slug %/]
		] [
			rejoin [site-url header/slug %/]
		]
	]

	entries-dir: (rejoin [system/options/path %entries/])
	templates-dir: (rejoin [system/options/path %templates/])

	;-- Even though I don't want the URL to have a path before the slug
	;-- on blog posts, I'd rather have them in their own directory on
	;-- the backend in the templates directory.
	file-for-template: function [header [object!]] [
		either find site-toplevel-slugs header/slug [
			rejoin [templates-dir %page/ header/slug ".html"]
		] [
			rejoin [templates-dir %post/ header/slug ".html"]
		]
	]

	;-- Optional: CSS included on every page
	css: [
;		http://hostilefork.com/media/google-code-prettify/prettify.css
;		http://hostilefork.com/media/google-code-prettify/reb4me-prettify.css
		http://hostilefork.com/media/default.css
		{
			#title h1 {
				padding-right: 80px;
			}

			#title {
    			min-height: 75px;
    			background-image: url(http://hostilefork.com/media/peeking-fork-80x75.png);
    			background-repeat: no-repeat;
    			background-position: right bottom;
			}

			/* We suppress the characters div from dialogue */
			#entry-characters {
				display: none;
			}
		}
	]

	;-- Optional: JS included on every page
	javascript: [
		http://ajax.googleapis.com/ajax/libs/jquery/1.10.1/jquery.min.js

		{
/* http://stackoverflow.com/questions/1402698/binding-arrow-keys-in-js-jquery */
$(document).keydown(function(e) {
	if (e.shiftKey || e.ctrlKey || e.altKey || e.metaKey)
		return;

	switch(e.which) {
		case 37: // left (up is 38)
			anchors = $("#prev > a");
			if (anchors.length > 0) {
				window.location.href = anchors.eq(0).attr('href');
			}
		break;

	case 39: // right (down is 40)
			anchors = $("#next > a");
			if (anchors.length > 0) {
				window.location.href = anchors.eq(0).attr('href');
			}
		break;

	default: return; // exit this handler for other keys
	}

	e.preventDefault(); // prevent the default action (scroll / move caret)
});
		}

		http://hostilefork.com/media/google-code-prettify/prettify.js
		http://hostilefork.com/media/google-code-prettify/lang-rebol.js

		{
$(document).ready(
	function(){
		window.prettyPrint && prettyPrint()
	}
);
		}
	]
]
