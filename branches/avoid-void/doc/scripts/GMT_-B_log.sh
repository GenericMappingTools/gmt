#!/bin/bash
#	$Id$
#
. ./functions.sh

gmtset MAP_GRID_PEN_PRIMARY thinnest,.
psbasemap -R1/1000/0/1 -JX3il/0.25i -B1f2g3p:"Axis Label":S -K -P > GMT_-B_log.ps
psbasemap -R -J -B1f2g3l:"Axis Label":S -O -K -Y0.85i >> GMT_-B_log.ps
psbasemap -R -J -B1f2g3:"Axis Label":S -O -Y0.85i >> GMT_-B_log.ps
