#!/bin/sh
#	$Id: GMT_lambert_az_rect.sh,v 1.2 2001-09-14 18:30:17 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE 0
pscoast -R0/-40/60/-10r -JA30/-30/4.5i -B30g30/15g15 -Dl -A500 -G200 -W0.25p -P > \
    GMT_lambert_az_rect.ps
