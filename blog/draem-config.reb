Rebol []

draem/set-config object compose [
	site-url: http://blog.hostilefork.com/
	site-name: "Hostilefork.com Blog"
	site-title: {HostileFork Blog}
	site-tagline: {Not actually *hostile* (just a bit irate.)}

	site-intro: {
		<img src="https://www.gravatar.com/avatar/4718212e95d7adea8c412379e7f542e9?s=128&d=identicon&r=PG" alt="The Fork" />
		<p>Currently blog.hostilefork.com is a scrape and rewrite of content that was previously on <a href="">hostilefork.com</a>. <i>Your patience is appreciated while the transition away from WordPress is underway.</i></a>
		<hr />
		<p>In addition to the master list below, you can browse the entries by <a href="{% url draems.views.tag_list %}">tag</a>.  Two early experimental features I've added are browsing by <a href="{% url draems.views.character_list %}">character</a> or with a <a href="{% url draems.views.timeline %}">timeline</a>.  I appreciate your feedback or suggestions, so do not hesitate to contact me!</p>
	}

	site-footer: {
	<div style="text-align: center;"><p>Please subscribe to the <img src="/media/feed-icon-14x14.png" alt="Feed Icon" /> <a href="http://hostilefork.com/feed/">Atom 1.0 Feed</a> or use <img src="/media/feedburner-icon-14x14.png" alt="Feedburner Icon" /> <a href="http://feeds.feedburner.com/realityhandbook">Feedburner</a> to receive updates as they are posted!!</p></div>
	}

	site-toplevel-slugs: [
		%about
		%albumist
		%blackhighlighter
		%hire-the-fork
		%hoist
		%legal
		%nocycle
		%nstate
		%openzoom-squared
		%rebmu
		%rubol
		%site-technical-details
		%thinker-qt
		%uscii
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

	file-from-header: function [header [object!]] [
		either find site-toplevel-slugs header/slug [
			rejoin [%page/ header/slug]
		] [
			rejoin [%post/ header/slug]
		]
	]

	;-- Required url-from-header hook
	url-from-header: function [header [object!]] [
		rejoin [site-url (file-from-header header)]
	]

	entries-dir: (rejoin [system/options/path %entries/])
	templates-dir: (rejoin [system/options/path %templates/])

	file-for-template: function [header [object!]] [
		rejoin [templates-dir (file-from-header header) ".html"]
	]
]