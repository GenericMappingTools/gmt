#!/bin/sh
#	$Id: GMT_mercator.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset DEGREE_FORMAT 1 BASEMAP_TYPE FANCY
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f30/a30f15 -Dc -A5000 -G0 -P > GMT_mercator.ps
