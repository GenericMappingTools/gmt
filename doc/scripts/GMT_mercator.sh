#!/bin/sh
#	$Id: GMT_mercator.sh,v 1.5 2009-02-15 20:22:05 remko Exp $
#

gmtset BASEMAP_TYPE fancy
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f30/a30f15 -Dc -A5000 -Gblack -P > GMT_mercator.ps
