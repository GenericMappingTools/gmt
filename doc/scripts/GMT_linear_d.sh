#!/bin/sh
#	$Id: GMT_linear_d.sh,v 1.5 2006-10-24 01:53:19 remko Exp $
#

gmtset GRID_CROSS_SIZE_PRIMARY 0.1i BASEMAP_TYPE FANCY PLOT_DEGREE_FORMAT ddd:mm:ssF
pscoast -R-55/305/-90/90 -Jx0.014id -B60g30f15/30g30f15WSen -Dc -A1000 -Glightgray -Wthinnest -P \
    > GMT_linear_d.ps
gmtset GRID_CROSS_SIZE_PRIMARY 0
