#!/bin/sh
#	$Id: GMT_lambert_az_rect.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset DEGREE_FORMAT 0 GRID_CROSS_SIZE 0
pscoast -R0/-40/60/-10r -JA30/-30/4.5i -B30g30/15g15 -Dl -A500 -G200 -W0.25p -P > \
    GMT_lambert_az_rect.ps
