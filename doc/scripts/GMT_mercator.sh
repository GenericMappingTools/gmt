#!/bin/bash
#	$Id: GMT_mercator.sh,v 1.9 2011-05-22 22:14:06 guru Exp $
#
. ./functions.sh

gmtset MAP_FRAME_TYPE fancy
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f30/a30f15 -Dc -A5000 -Gred -P > GMT_mercator.ps
