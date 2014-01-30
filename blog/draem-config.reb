Rebol []

draem/set-config object compose [
	site-url: http://blog.hostilefork.com/
	site-name: "Hostilefork.com Blog"
	site-title: {HostileFork Blog}
	site-tagline: {Not actually *hostile* (just a bit irate.)}

	site-intro: {
		<img src="http://i.stack.imgur.com/VWHTU.png" alt="The Fork" />
		<p>Currently blog.hostilefork.com is a scrape and rewrite of content that was previously on <a href="">hostilefork.com</a>. <i>Your patience is appreciated while the transition from WordPress is underway.</i></a>
		<hr />
		<p>The project pages are getting new homes too.  So go check out <a href="http://blackhighlighter.org">BlackHighlighter</a>, <a href="http://albumist.org">Albumist</a>, <a href="http://uscii.hostilefork.com">USCII</a>, <a href="http://thinker-qt.hostilefork.com">Thinker-Qt</a>, <a href="http://hostilefork.com/rebmu/">Rebmu</a>, <a href="http://hostilefork.com/rubol/">Rubol</a>, <a href="http://blog.hostilefork.com/imagination-squared-plus-openzoom/">OpenZoom-Squared</a>, and many more to be added as the pages here get put together better.</p>
		<hr />
		<p>In addition to the master list below, you can browse the entries by <a href="{% url draems.views.tag_list %}">tag</a>.  There is some preliminary support for navigating the entries with the left and right arrow keys, and all the comments from the old site have been imported into Disqus.  I appreciate your feedback or suggestions, so do not hesitate to contact me!</p>		
	}

	site-footer: {
	<div style="text-align: center;"><p>Please subscribe to the <img src="http://hostilefork.com/media/feed-icon-14x14.png" alt="Feed Icon" /> <a href="http://blog.hostilefork.com/feed/">Atom 1.0 Feed</a> or use <img src="http://blog.hostilefork.com/media/feedburner-icon-14x14.png" alt="Feedburner Icon" /> <a href="http://feeds.feedburner.com/hostilefork">Feedburner</a> to receive updates as they are posted!!</p></div>
	}

	site-toplevel-slugs: [
		%homepage
		%about
		%albumist
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
			rejoin [site-url %page/ header/slug %/]
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
]