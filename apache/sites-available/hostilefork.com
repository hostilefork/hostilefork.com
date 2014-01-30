# Should be able to just say *:80 but that is unpredictable...
# http://www.webmasterworld.com/apache/3282118.htm
<VirtualHost 72.249.123.186:80>

        ServerAdmin brian@hostilefork.com
        DocumentRoot /var/www/hostilefork.com/
        ServerName hostilefork.com
        ServerAlias *.hostilefork.com

        # Proxy subdomains to subdirectories when hosting a subdomain on the
        # same server as the main domain
        #
        # http://serverfault.com/questions/547262/

        # [NC] on a rule means "no case" (case-insensitive patterns)
        # A series of RewriteConds are ANDed together by default, unless you use [OR]
        # Combine flags with commas (e.g. [NC,OR])
        RewriteEngine on

	# REVIEW: rules are generally supposed to have [...,L] with the L meaning
	# "last rule to process".  Which of these should be "last"?

	#
	# Support for the old WordPress URL scheme
	# Note that some of the slugs were too wordy and rewritten
	# Google had already indexed my blog post bad urls :-(
	# So this will cause a redirect which may redirect again...
	#
	# I don't want the new blog to tolerate underscores, so any
	# old underscore-based URLs must be handled explicitly.
	#
	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/[0-9]{4}/[0-9]{2}/[0-9]{2}/modern_cpp_or_modern_art*.*$
	RewriteRule ^.*$ http://blog\.hostilefork\.com/modern-cpp-or-modern-art/ [R=301]

	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/[0-9]{4}/[0-9]{2}/[0-9]{2}/when-should-one-use-const_cast-anyway*.*$
	RewriteRule ^.*$ http://blog\.hostilefork\.com/when-should-one-use-const-cast/ [R=301]

	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/[0-9]{4}/[0-9]{2}/[0-9]{2}/[a-z0-9\-\+]+/*.*$
	RewriteRule ^/[0-9]{4}/[0-9]{2}/[0-9]{2}/([a-z0-9\-\+]+)/*.*$ http://blog\.hostilefork\.com/$1 [R=301]

	#
	# I got rid of "categories"
	#
	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/category/[a-z0-9\+\-]+/*.*$
	RewriteRule ^/category/([a-z0-9\+\-]+)/*.*$ http://blog\.hostilefork\.com/tag/$1 [R=301]

	#
	# Wordpress tag rules
	#
	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/tag/[a-z0-9\+\-]+/*.*$
	RewriteRule ^/tag/([a-z0-9\+\-]+)/*.*$ http://blog\.hostilefork\.com/tag/$1 [R=301]

	#
	# Some weird RPC that Google picked up.
	#
	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/xmlrpc\.php.*$
        RewriteRule ^(.*)$ - [G]

	#
	# Wordpress login page that hackers were always trying to test with simple passwords
	#
	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/wp-login\.php.*$
        RewriteRule ^(.*)$ - [G]

	#
	# I got rid of the WP form of date indexing, not currently supporting it
	# Return http 410, G for "Gone".  Same for page indexing by number.
	#
        RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/[0-9]{4}/[0-9]{0,2}/*$
        RewriteRule ^(.*)$ - [G]

	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/page/[0-9]{1,2}/*$
        RewriteRule ^(.*)$ - [G]
	
	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/[0-9]{4}/page/./*$
        RewriteRule ^(.*)$ - [G]

	#
	# All slugs were renamed to be no more than five words separated by hyphens
	# This rename include keeps them redirecting
	#
	Include /etc/apache2/includes/renames/build/renames.conf

	#
	# USCII demo has moved
	#
	RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
	RewriteCond %{REQUEST_URI} ^/demos/uscii-5x7-english-c0/[a-z]+\.html
	RewriteRule ^/demos/uscii-5x7-english-c0/([a-z]+)\.html http://uscii.hostilefork.com/$1/ [R=301]

        #
        # Provide HTTP redirect "[R]" for network-external requests
        # For permanent redirects, use [R=301], but note cache concerns:
        # http://getluky.net/2010/12/14/301-redirects-cannot-be-undon/
        #
        RewriteCond %{HTTP_HOST} ^blog\.hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/post/.*$
        RewriteRule ^/post/(.*)$ http://blog\.hostilefork\.com/$1 [R=301]

	#
	# Tag page for "stackoverflow" got popular but I  prefer the tag
	# "stackexchange", same for cplusplus to c++.  Arecibo is a weird
	# tag, so I just redirect to uscii.  Other fixes.
	#
        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/stackoverflow/.*$
        RewriteRule ^.*$ http://blog\.hostilefork\.com/tag/stackexchange/ [R=301]

        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/cplusplus/.*$
        RewriteRule ^.*$ http://blog\.hostilefork\.com/tag/c++/ [R=301]

        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/arecibo/.*$
        RewriteRule ^.*$ http://uscii\.hostilefork\.com/ [R=301]

        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/misc/.*$
        RewriteRule ^.*$ http://blog\.hostilefork\.com/ [R=301]

        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/uncategorized/.*$
        RewriteRule ^.*$ http://blog\.hostilefork\.com/ [R=301]

        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/fundable/.*$
        RewriteRule ^.*$ http://blog\.hostilefork\.com/tag/kickstarter/ [R=301]

        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/parse/.*$
        RewriteRule ^.*$ http://blog\.hostilefork\.com/tag/parsing/ [R=301]

        RewriteCond %{HTTP_HOST} ^(blog\.)?hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/tag/c/.*$
        RewriteRule ^.*$ http://blog\.hostilefork\.com/tag/c++/ [R=301]

	#
	# Put what used to be project pages on their own subdomains
	#
	Include /etc/apache2/includes/pages/build/pages.conf

	#
	# OpenZoom-Squared is not a project/page/subdomain
	# I consider it a blog entry, since I'm not taking it further
	# also openzoom-squared.hostilefork.com is a long and ugly subdomain
	#
	
        RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/openzoom-squared/.*$ [NC]
        RewriteRule ^.*$ http://blog\.hostilefork\.com/imagination-squared-plus-openzoom/ [R=301]

	# These aliases are to retain compatibility with media
	# locations that were on hostilefork.com.  Ideally
	# I would go through and redirect them but for now
	# I'd like to keep as many of them working as
	# possible to not break google image searches/etc.

	Alias /shared/ /var/www/hostilefork.com/media/shared/
	DirectoryIndex index.html

	# Text files run through Django by default... we want to redirect
        RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/shared/rebol/whitespace-output.txt$ [NC]
        RewriteRule ^.*$ https://gist\.github\.com/hostilefork/8628820 [R=301]

        RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/shared/rebol/tag_proposal.txt$ [NC]
        RewriteRule ^.*$ http://www\.rebol\.net/wiki/Extended_tag_proposal [R=301]

	#
	# I want to give all the old wordpress uploads better names and locations
	# but it's not a priority right now.
	#
	Alias /wp-content/ /var/www/hostilefork.com/media/wp-content/

	#
	# Redoing the Jing screencasts hosted on hostilefork.com to YouTube HD
	#	
        RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/wp-content/uploads/2007/11/explorer_undo_bug\.swf.*$ [NC]
        RewriteRule ^.*$ http://blog\.hostilefork\.com/undo-single-user-event/? [R=301]

        RewriteCond %{HTTP_HOST} ^hostilefork\.com [NC]
        RewriteCond %{REQUEST_URI} ^/wp-content/uploads/2007/11/lost_focus_demo\.swf.*$ [NC]
        RewriteRule ^.*$ http://blog\.hostilefork\.com/lost-focus-placeholder/? [R=301]

	Alias /projects/ /var/www/hostilefork.com/media/projects/
	Alias /favicon.ico /var/www/hostilefork.com/media/favicon.ico

        <Directory /var/lib/awstats>
                Options None
                AllowOverride None
                Order allow,deny
                Allow from all
        </Directory>

        <Directory /usr/share/awstats/icon>
                Options None
                AllowOverride None
                Order allow,deny
                Allow from all
        </Directory>

	<Location "/">
		SetHandler python-program
		PythonHandler django.core.handlers.modpython
		SetEnv DJANGO_SETTINGS_MODULE project.settings
#		PythonOption django.root /
		PythonPath "['/var/www/hostilefork.com/','/var/www/hostilefork.com/project/'] + sys.path" 
		PythonDebug On
	</Location>

	#
	# "Django doesn't serve media files itself; it usually expects you to have a different
	#  webserver serving these files. In our case though, we want Apache to handle it, so we 
	#  need to turn off mod_python for some parts of the site."
	#
	# http://jeffbaier.com/articles/installing-django-on-an-ubuntu-linux-server/
	#
	<Location "/media">
		SetHandler None
	</Location>
	<Location "/admin_media">
		SetHandler None
	</Location>
	<Location "/phpmyadmin">
		SetHandler None
	</Location>
	<locationmatch ".(jpg|gif|png|pdf|ico|jar|css|js|html)$">
		SetHandler None
	</Locationmatch>

        Alias /awstats-icon/ /usr/share/awstats/icon/
        ScriptAlias /stats /usr/lib/cgi-bin/awstats.pl
        ScriptAlias /awstats.pl /usr/lib/cgi-bin/awstats.pl

        ErrorLog /var/www/hostilefork.com/logs/error.log
        CustomLog /var/www/hostilefork.com/logs/access.log common
        CustomLog /var/www/hostilefork.com/logs/access-combined.log combined
</VirtualHost>
