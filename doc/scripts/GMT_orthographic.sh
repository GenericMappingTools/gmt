#!/bin/sh
#	$Id: GMT_orthographic.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R0/360/-90/90 -JG-75/40/4.5i -B15g15 -Dc -A5000 -G0 -P > GMT_orthographic.ps
