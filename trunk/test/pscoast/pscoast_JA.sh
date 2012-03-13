#!/bin/bash
#
#	$Id$
# Make sure when fixed it works for all resolutions -D?

. ./functions.sh
header "Test pscoast for JA plot of Germany"

pscoast -JA13:25/52:31/10/7i -Rg -Gred -Sblue -Dl -P > $ps

pscmp
