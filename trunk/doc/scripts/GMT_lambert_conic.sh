#!/bin/sh
#	$Id: GMT_lambert_conic.sh,v 1.2 2001-09-14 18:30:17 pwessel Exp $
#

gmtset BASEMAP_TYPE FANCY PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE 0.05i
pscoast -R-130/-70/24/52 -Jl-100/35/33/45/1:50000000 -B10g5 -Dl -N1/1p -N2/0.5p -A500 -G200 \
   -W0.25p -P > GMT_lambert_conic.ps
gmtset GRID_CROSS_SIZE 0
