#!/bin/sh
#	$Id: GMT_grinten.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JV180/4i -Bg30/g15 -Dc -Glightgray -A10000 -W0.25p -P > GMT_grinten.ps
