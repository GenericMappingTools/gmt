#!/bin/sh
#	$Id: GMT_equidistant_conic.sh,v 1.3 2004-04-10 17:19:14 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE 0.05i
pscoast -R-88/-70/18/24 -JD-79/21/19/23/4.5i -B5g1 -Di -N1/1p -Glightgray \
   -W0.25p -P > GMT_equidistant_conic.ps
gmtset GRID_CROSS_SIZE 0
