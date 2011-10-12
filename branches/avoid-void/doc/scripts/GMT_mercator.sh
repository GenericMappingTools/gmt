#!/bin/bash
#	$Id$
#
. ./functions.sh

gmtset MAP_FRAME_TYPE fancy
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f15/a30f15 -Dc -A5000 -Gred -P > GMT_mercator.ps
