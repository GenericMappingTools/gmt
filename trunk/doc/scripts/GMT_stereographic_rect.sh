#!/bin/bash
#	$Id: GMT_stereographic_rect.sh,v 1.8 2011-02-28 00:58:02 remko Exp $
#
. functions.sh

gmtset OBLIQUE_ANNOTATION 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -Glightgray -Wthinnest -P \
	> GMT_stereographic_rect.ps
