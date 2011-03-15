#!/bin/bash
#	$Id: GMT_lambert_conic.sh,v 1.10 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

gmtset MAP_FRAME_TYPE FANCY FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0.05i
pscoast -R-130/-70/24/52 -Jl-100/35/33/45/1:50000000 -B10g5 -Dl -N1/thick,red -N2/thinner \
	-A500 -Gtan -Wthinnest,white -Sblue -P > GMT_lambert_conic.ps
