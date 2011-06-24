#!/bin/bash
#	$Id: GMT_mercator.sh,v 1.10 2011-06-24 03:43:00 guru Exp $
#
. ./functions.sh

gmtset MAP_FRAME_TYPE fancy
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f15/a30f15 -Dc -A5000 -Gred -P > GMT_mercator.ps
