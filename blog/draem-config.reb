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

	<!-- This is the Goolge Universal Analytics Code -->
	<script>
	  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
	  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
	  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
	  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');

	  ga('create', 'UA-2926272-2', 'hostilefork.com');
	  ga('send', 'pageview');
	</script>

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