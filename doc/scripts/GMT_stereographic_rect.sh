#!/bin/sh
#	$Id: GMT_stereographic_rect.sh,v 1.4 2004-04-10 17:19:14 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ss OBLIQUE_ANNOTATION 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -Glightgray -W.25p -P > \
    GMT_stereographic_rect.ps
