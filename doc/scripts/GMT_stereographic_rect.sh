#!/bin/sh
#	$Id: GMT_stereographic_rect.sh,v 1.2 2001-09-14 18:30:17 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ss OBLIQUE_ANOTATION 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -G200 -W.25p -P > \
    GMT_stereographic_rect.ps
