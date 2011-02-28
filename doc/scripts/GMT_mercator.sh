#!/bin/bash
#	$Id: GMT_mercator.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

gmtset BASEMAP_TYPE fancy
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f30/a30f15 -Dc -A5000 -Gblack -P > GMT_mercator.ps
