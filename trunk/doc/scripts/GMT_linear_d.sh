#!/bin/bash
#	$Id: GMT_linear_d.sh,v 1.9 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

gmtset MAP_GRID_CROSS_SIZE_PRIMARY 0.1i MAP_FRAME_TYPE FANCY FORMAT_GEO_MAP ddd:mm:ssF
pscoast -Rg-55/305/-90/90 -Jx0.014i -B60g30f15/30g30f15WSen -Dc -A1000 -Glightbrown -Wthinnest -P \
	-Slightblue > GMT_linear_d.ps
