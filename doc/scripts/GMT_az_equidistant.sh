#!/bin/sh
#	$Id: GMT_az_equidistant.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R0/360/-90/90 -JE-100/40/4.5i -B15g15 -Dc -A10000 -G200 -W0.25p -P > GMT_az_equidistant.ps
