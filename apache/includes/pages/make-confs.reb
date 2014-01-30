Rebol []

space: #" "

subdomains: [
	benzene
	copylight
	draem
	hoist
	methyl
	nocycle
	nstate
	pixelcad
	rebmu
	sourcecad
	thinker-qt
	titlewait
	trackcad
	uscii
	jquery-numband

	albumist
	blackhighlighter
]

pages-conf: copy []

foreach s subdomains [
	sub: to string! s	

	conf-text: rejoin [{
	#
	# Do not allow direct access of subdomain path on that
	# subdomain; e.g. prohibit uscii.hostilefork.com/uscii/*
	#
	RewriteCond %{HTTP_HOST} ^^} sub {\.hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^^/} sub {/.*$ [NC]
	RewriteRule ^^/(.*)$ /$1 [R=404,L]

	#
	# Media directories are just hosted in respective subdirectories
	# of hostilefork.com/media/ - e.g. uscii.hostilefork.com/media/*
	# will be retrieving the media/uscii/* under the hood by
	# ussing Pass-Through "[PT]".
	#
        RewriteCond %{HTTP_HOST} ^^} sub {\.hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^^/media/.*$ [NC]
	RewriteRule ^^/media/(.*) /media/} sub {/$1 [PT]

	#
        # Provide HTTP redirect "[R]" for network-external requests
        # For permanent redirects, use [R=301], but note cache concerns:
        # http://getluky.net/2010/12/14/301-redirects-cannot-be-undon/
        #
        RewriteCond %{HTTP_HOST} ^^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^^/} sub {/.*$ [NC]
        RewriteRule ^^/} sub {/(.*)$ http://} sub {\.hostilefork.com/$1 [R=301]

        # Proxy subdomains to subdirectories when hosting a subdomain on the
        # same server as the main domain
        #
        # http://serverfault.com/questions/547262/

        #
        # Pass-Through "[PT]" to subpath URL for subdomain requests
        # (Assumes that foo.example.com/foo would serve the same
        # content as example.com/foo, if not for the above rule)
        #
        RewriteCond %{HTTP_HOST} ^^} sub {\.hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^^(?!/} sub {).*$ [NC]
        RewriteRule ^^/(.*)$ /} sub {/$1 [PT,L]
	}
	]

	filename: rejoin [system/options/path %build/ sub {.conf}]
	write filename conf-text
	append pages-conf rejoin [tab {Include} space to string! filename]
]

write/lines rejoin [system/options/path %build/pages.conf] pages-conf
