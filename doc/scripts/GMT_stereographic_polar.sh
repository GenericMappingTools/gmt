#!/bin/sh
#	$Id: GMT_stereographic_polar.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset DEGREE_FORMAT 1
pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -Ba10g5/5g5 -Dl -A250 -G0 -P > GMT_stereographic_polar.ps
