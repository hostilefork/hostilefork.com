Rebol [
	Title: "Make Hostilefork Blog"
	Description: {
		This is the script for building the static template files behind
		the website blog.hostilefork.com.  It requires that you have
		a subdirectory called "draem" containing the Draem builder:

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

draem/set-config object compose [
	site-url: http://blog.hostilefork.com/
	site-name: "Hostilefork.com Blog"

	valid-categories: [
		post
		page
	]

	;-- Required url-from-header hook	
	url-from-header: function [header [object!]] [

		;-- NOTE: Somehow Google crawled my *unpublished* site and picked up
		;-- an unfinished URL scheme.  So now I have to redirect those URLs
		;-- as well.  Or decide I don't care if they break, as no one but
		;-- them link to it.

		rejoin [
			site-url
			stringify/dashes header/category
			{/}
			header/slug
			;-- terminal slash required? {/}
		]
	]

	entries-dir: (rejoin [system/options/path %entries/])
	templates-dir: (rejoin [system/options/path %templates/])
]

draem/make-site
