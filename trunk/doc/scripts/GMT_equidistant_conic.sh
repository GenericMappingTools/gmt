#!/bin/sh
#	$Id: GMT_equidistant_conic.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset DEGREE_FORMAT 3 GRID_CROSS_SIZE 0.05i
pscoast -R-88/-70/18/24 -JD-79/21/19/23/4.5i -B5g1 -Di -N1/1p -G200 \
   -W0.25p -P > GMT_equidistant_conic.ps
gmtset GRID_CROSS_SIZE 0
