#!/bin/sh
#	$Id: GMT_equidistant_conic.sh,v 1.5 2006-10-24 01:53:19 remko Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE_PRIMARY 0.05i
pscoast -R-88/-70/18/24 -JD-79/21/19/23/4.5i -B5g1 -Di -N1/thick -Glightgray \
   -Wthinnest -P > GMT_equidistant_conic.ps
gmtset GRID_CROSS_SIZE_PRIMARY 0
