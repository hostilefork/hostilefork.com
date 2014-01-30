Rebol []

space: #" "

renamed-slugs: load %renamed-slugs.reb

conf-text: copy {
	#
        # Provide HTTP redirect "[R]" for network-external requests
        # For permanent redirects, use [R=301], but note cache concerns:
        # http://getluky.net/2010/12/14/301-redirects-cannot-be-undon/
        #
}

foreach [old-name new-name] renamed-slugs [

	append conf-text rejoin [{
	        RewriteCond %{HTTP_HOST} ^^blog\.hostilefork\.com [NC]
	        RewriteCond %{REQUEST_URI} ^^} to string! old-name {/.*$ [NC]
	        RewriteRule ^^} to string! old-name {/.*$} space {http://blog.hostilefork.com} to string! new-name {/ [R]
	}]

	;-- No idea how, but google somehow picked up hostilefork.com/old-slug patterns too!
	append conf-text rejoin [{
	        RewriteCond %{HTTP_HOST} ^^hostilefork\.com [NC]
	        RewriteCond %{REQUEST_URI} ^^} to string! old-name {/.*$ [NC]
	        RewriteRule ^^} to string! old-name {/.*$} space {http://blog.hostilefork.com} to string! new-name {/ [R]
	}]
]

filename: rejoin [system/options/path %build/renames.conf]
write filename conf-text
print rejoin [tab tab {Include} space to string! filename]
