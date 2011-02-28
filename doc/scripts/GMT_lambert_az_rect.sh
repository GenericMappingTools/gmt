#!/bin/bash
#	$Id: GMT_lambert_az_rect.sh,v 1.7 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE_PRIMARY 0
pscoast -R0/-40/60/-10r -JA30/-30/4.5i -B30g30/15g15 -Dl -A500 -Glightgray -Wthinnest -P \
	> GMT_lambert_az_rect.ps
