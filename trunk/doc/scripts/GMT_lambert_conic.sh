#!/bin/sh
#	$Id: GMT_lambert_conic.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset BASEMAP_TYPE FANCY DEGREE_FORMAT 3 GRID_CROSS_SIZE 0.05i
pscoast -R-130/-70/24/52 -Jl-100/35/33/45/1:50000000 -B10g5 -Dl -N1/1p -N2/0.5p -A500 -G200 \
   -W0.25p -P > GMT_lambert_conic.ps
gmtset GRID_CROSS_SIZE 0
