#!/bin/sh
#	$Id: GMT_stereographic_polar.sh,v 1.3 2004-04-10 17:19:14 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ss
pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -Ba10g5/5g5 -Dl -A250 -Gblack -P > GMT_stereographic_polar.ps
