#!/bin/sh
#	$Id: GMT_equidistant_conic.sh,v 1.2 2001-09-14 18:30:17 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE 0.05i
pscoast -R-88/-70/18/24 -JD-79/21/19/23/4.5i -B5g1 -Di -N1/1p -G200 \
   -W0.25p -P > GMT_equidistant_conic.ps
gmtset GRID_CROSS_SIZE 0
