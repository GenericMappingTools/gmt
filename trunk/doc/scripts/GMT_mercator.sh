#!/bin/bash
#	$Id: GMT_mercator.sh,v 1.8 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

gmtset MAP_FRAME_TYPE fancy
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f30/a30f15 -Dc -A5000 -Gblack -P > GMT_mercator.ps
