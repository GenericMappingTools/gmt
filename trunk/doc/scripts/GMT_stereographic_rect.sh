#!/bin/sh
#	$Id: GMT_stereographic_rect.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset DEGREE_FORMAT 1 OBLIQUE_ANOTATION 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -G200 -W.25p -P > \
    GMT_stereographic_rect.ps
