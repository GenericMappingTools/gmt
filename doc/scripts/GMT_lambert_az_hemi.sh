#!/bin/sh
#	$Id: GMT_lambert_az_hemi.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JA280/30/3.5i -B30g30/15g15 -Dc -A1000 -Gblack -P > GMT_lambert_az_hemi.ps
