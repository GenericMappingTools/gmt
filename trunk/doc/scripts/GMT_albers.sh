#!/bin/sh
#	$Id: GMT_albers.sh,v 1.3 2004-04-12 21:41:47 pwessel Exp $
#

gmtset GRID_CROSS_SIZE_PRIMARY 0
pscoast -R110/140/20/35 -JB125/20/25/45/5i -B10g5 -Dl -Glightgray -W0.25p -A250 -P > GMT_albers.ps
