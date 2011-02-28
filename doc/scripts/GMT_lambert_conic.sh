#!/bin/bash
#	$Id: GMT_lambert_conic.sh,v 1.9 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

gmtset BASEMAP_TYPE FANCY PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE_PRIMARY 0.05i
pscoast -R-130/-70/24/52 -Jl-100/35/33/45/1:50000000 -B10g5 -Dl -N1/thick -N2/thinner -A500 \
	-Glightgray -Wthinnest -P > GMT_lambert_conic.ps
