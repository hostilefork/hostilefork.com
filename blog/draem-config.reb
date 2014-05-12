Rebol []

draem/set-config object compose [
	site-url: http://blog.hostilefork.com/
	site-name: "Hostilefork.com Blog"
	site-title: {HostileFork Blog}
	site-tagline: {Not actually *hostile* (just a bit irate.)}

	site-intro: {
		<img src="http://i.stack.imgur.com/VWHTU.png" alt="The Fork" />
		<p>Currently blog.hostilefork.com is a scrape and rewrite of content that was previously on the main page of hostilefork.com.  If the pages look a bit primitive, that's because it was created with a fledgling static site builder of my own design: <a href="https://github.com/hostilefork/draem">"Draem"</a>.  Rethinking all the URLs while keeping all the old ones forwarding has been enough of a task on its own, so please report anything you see broken.</a>
		<hr />
		<p>The project pages are starting to get better summaries, logos, and videos.  For the moment, the portal page for that is what's being served on the homepage at <b><a href="http://hostilefork.com">hostilefork.com</a></b>.</p>
		<hr />
		<p>In addition to the master list of blog entries below, you can browse by <a href="{% url 'draems.views.tag_list' %}">tag</a>.  There is some preliminary support for navigating the entries with the left and right arrow keys, and all the comments from the old site have been imported into Disqus.  I appreciate your feedback or suggestions, so do not hesitate to contact me!</p>		
	}

	site-footer: {
	<div style="text-align: center;"><p>Please subscribe to the <img src="http://hostilefork.com/media/feed-icon-14x14.png" alt="Feed Icon" /> <a href="http://blog.hostilefork.com/feed/">Atom 1.0 Feed</a> or use <img src="http://blog.hostilefork.com/media/feedburner-icon-14x14.png" alt="Feedburner Icon" /> <a href="http://feeds.feedburner.com/hostilefork">Feedburner</a> to receive updates as they are posted!!</p></div>

  	<div id="disqus_thread"></div>
	<script type="text/javascript">
	/* * * CONFIGURATION VARIABLES: EDIT BEFORE PASTING INTO YOUR WEBPAGE * * */
	var disqus_shortname = 'hostilefork'; // required: replace example with your forum shortname

	/* * * DON'T EDIT BELOW THIS LINE * * */
	(function() {
	var dsq = document.createElement('script'); dsq.type = 'text/javascript'; dsq.async = true;
	dsq.src = '//' + disqus_shortname + '.disqus.com/embed.js';
	(document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(dsq);
	})();
	</script>
	<noscript>Please enable JavaScript to view the <a href="http://disqus.com/?ref_noscript">comments powered by Disqus.</a></noscript>
	<a href="http://disqus.com" class="dsq-brlink">comments powered by <span class="logo-disqus">Disqus</span></a>
	}

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
		%/media/google-code-prettify/prettify.css
		%/media/google-code-prettify/reb4me-prettify.css
	]

	;-- Optional: JS included on every page
	javascript: [
		http://ajax.googleapis.com/ajax/libs/jquery/1.10.1/jquery.min.js

		{
/* http://stackoverflow.com/questions/1402698/binding-arrow-keys-in-js-jquery */
$(document).keydown(function(e) {
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

		%/media/google-code-prettify/prettify.js
		%/media/google-code-prettify/lang-rebol.js

		{
$(document).ready(
	function(){
		window.prettyPrint && prettyPrint()
	}
);
		}
	]
]
