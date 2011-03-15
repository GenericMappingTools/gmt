#!/bin/bash
#	$Id: GMT_mercator.sh,v 1.7 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

gmtset MAP_FRAME_TYPE fancy
pscoast -R0/360/-70/70 -Jm1.2e-2i -Ba60f30/a30f15 -Dc -A5000 -Gblack -P > GMT_mercator.ps
