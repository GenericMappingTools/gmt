#!/bin/sh
#	$Id: GMT_linear_d.sh,v 1.4 2004-04-12 21:41:47 pwessel Exp $
#

gmtset GRID_CROSS_SIZE_PRIMARY 0.1i BASEMAP_TYPE FANCY PLOT_DEGREE_FORMAT ddd:mm:ssF
pscoast -R-55/305/-90/90 -Jx0.014id -B60g30f15/30g30f15WSen -Dc -A1000 -Glightgray -W0.25p -P \
    > GMT_linear_d.ps
gmtset GRID_CROSS_SIZE_PRIMARY 0
