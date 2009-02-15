#!/bin/sh
#	$Id: GMT_stereographic_polar.sh,v 1.4 2009-02-15 20:22:05 remko Exp $
#

pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -Ba10g5/5g5 -Dl -A250 -Gblack -P > GMT_stereographic_polar.ps
