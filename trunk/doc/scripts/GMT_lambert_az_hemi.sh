#!/bin/sh
#	$Id: GMT_lambert_az_hemi.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R0/360/-90/90 -JA280/30/3.5i -B30g30/15g15 -Dc -A1000 -G0 -P > GMT_lambert_az_hemi.ps
