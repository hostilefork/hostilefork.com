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

do %draem-config.reb

draem/make-site
