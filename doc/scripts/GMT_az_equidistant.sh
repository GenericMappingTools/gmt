#!/bin/sh
#	$Id: GMT_az_equidistant.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JE-100/40/4.5i -B15g15 -Dc -A10000 -Glightgray -W0.25p -P > GMT_az_equidistant.ps
