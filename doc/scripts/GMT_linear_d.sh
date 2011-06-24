#!/bin/bash
#	$Id: GMT_linear_d.sh,v 1.13 2011-06-24 03:43:00 guru Exp $
#
. ./functions.sh

gmtset MAP_GRID_CROSS_SIZE_PRIMARY 0.1i MAP_FRAME_TYPE FANCY FORMAT_GEO_MAP ddd:mm:ssF
pscoast -Rg-55/305/-90/90 -Jx0.014i -BagfWSen -Dc -A1000 -Glightbrown -Wthinnest -P \
	-Slightblue > GMT_linear_d.ps
