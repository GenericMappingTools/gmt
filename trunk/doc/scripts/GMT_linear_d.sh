#!/bin/sh
#	$Id: GMT_linear_d.sh,v 1.6 2006-10-30 02:59:36 remko Exp $
#

gmtset GRID_CROSS_SIZE_PRIMARY 0.1i BASEMAP_TYPE FANCY PLOT_DEGREE_FORMAT ddd:mm:ssF
pscoast -Rg-55/305/-90/90 -Jx0.014i -B60g30f15/30g30f15WSen -Dc -A1000 -Glightgray -Wthinnest -P \
    > GMT_linear_d.ps
gmtset GRID_CROSS_SIZE_PRIMARY 0
