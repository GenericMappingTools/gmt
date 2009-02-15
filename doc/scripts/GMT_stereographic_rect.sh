#!/bin/sh
#	$Id: GMT_stereographic_rect.sh,v 1.7 2009-02-15 20:22:05 remko Exp $
#

gmtset OBLIQUE_ANNOTATION 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -Glightgray -Wthinnest -P \
	> GMT_stereographic_rect.ps
