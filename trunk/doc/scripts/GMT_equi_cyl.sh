#!/bin/sh
#	$Id: GMT_equi_cyl.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JQ180/4.5i -B60f30g30 -Dc -A5000 -Gblack -P > GMT_equi_cyl.ps
