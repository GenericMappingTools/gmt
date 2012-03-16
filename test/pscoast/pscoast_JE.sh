#!/bin/bash
#
#	$Id$
# Make sure when fixed it works for all resolutions -D?

header "Test pscoast for JE plot of Germany"

pscoast -JE13:25/52:31/10/7i -Rg -Gred -Sblue -Dl -P > $ps

pscmp
