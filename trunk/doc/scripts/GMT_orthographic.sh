#!/bin/sh
#	$Id: GMT_orthographic.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JG-75/40/4.5i -B15g15 -Dc -A5000 -Gblack -P > GMT_orthographic.ps
