#!/bin/sh
#	$Id: GMT_linear_d.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset GRID_CROSS_SIZE 0.1i BASEMAP_TYPE FANCY DEGREE_FORMAT 3
pscoast -R-55/305/-90/90 -Jx0.014id -B60g30f15/30g30f15WSen -Dc -A1000 -G200 -W0.25p -P \
    > GMT_linear_d.ps
gmtset GRID_CROSS_SIZE 0
