#!/bin/sh
#	$Id: GMT_grinten.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R0/360/-90/90 -JV180/4i -Bg30/g15 -Dc -G200 -A10000 -W0.25p -P > GMT_grinten.ps
