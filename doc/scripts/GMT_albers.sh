#!/bin/sh
#	$Id: GMT_albers.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset GRID_CROSS_SIZE 0
pscoast -R110/140/20/35 -JB125/20/25/45/5i -B10g5 -Dl -G200 -W0.25p -A250 -P > GMT_albers.ps
