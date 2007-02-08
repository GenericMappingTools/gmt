#!/bin/sh
#	$Id: GMT_stereographic_rect.sh,v 1.6 2007-02-08 21:46:28 remko Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ss OBLIQUE_ANNOTATION 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -Glightgray -Wthinnest -P \
	> GMT_stereographic_rect.ps
