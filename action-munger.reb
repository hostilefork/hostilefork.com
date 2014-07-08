Rebol []

do %draem/draem.reb

do %draem-config.reb

draem/load-entries

;-- We want to convert all the non-HTML tagged content strings 
;-- to simiplified markdown.
tagify-actions: function [blk [any-block!]] [
	;-- leave html alone
	if 'html = first blk [exit]
		
	pos: blk
	while [not tail? pos] [
		item: first pos
		if block? item [
			case [
                all [
                        set-word? first item
                        paren? second item
                ] [
                		probe item
                        assert [string? first second item]
                        tag-action: to tag! first second item
                        change next item tag-action
                ]

                true [tagify-actions item]
            ]
        ]
		++ pos
	]	
]

foreach entry draem/entries [
	tagify-actions entry/content
]

draem/save-entries %munged/
