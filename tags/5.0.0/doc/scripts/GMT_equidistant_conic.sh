#!/bin/bash
#	$Id$
#
. ./functions.sh

gmtset FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0.05i
pscoast -R-88/-70/18/24 -JD-79/21/19/23/4.5i -Bag -Di -N1/thick,red -Glightgreen \
	-Wthinnest -P > GMT_equidistant_conic.ps
