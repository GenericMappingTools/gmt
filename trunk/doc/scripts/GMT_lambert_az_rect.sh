#!/bin/sh
#	$Id: GMT_lambert_az_rect.sh,v 1.4 2004-04-12 21:41:47 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE_PRIMARY 0
pscoast -R0/-40/60/-10r -JA30/-30/4.5i -B30g30/15g15 -Dl -A500 -Glightgray -W0.25p -P > \
    GMT_lambert_az_rect.ps
