Rebol [
	Title: "Make HostileFork"
	Description: {
		This is the script for building the static template files behind
		the website realityhandbook.org.  It requires that you have
		a subdirectory called "draem" containing the Draem builder:

			https://github.com/hostilefork/draem
		
		See the Draem documentation (and/or source) for config options.
	}

	Author: "hostilefork"
	Home: https://github.com/hostilefork/hostilefork.com
	License: 'bsd

	Date: 20-Oct-2010
	Version: 0.3.0.4

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
