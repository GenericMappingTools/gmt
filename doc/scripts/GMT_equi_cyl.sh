#!/bin/sh
#	$Id: GMT_equi_cyl.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R0/360/-90/90 -JQ180/4.5i -B60f30g30 -Dc -A5000 -G0 -P > GMT_equi_cyl.ps
