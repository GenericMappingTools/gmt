#!/bin/sh
#	$Id: GMT_albers.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

gmtset GRID_CROSS_SIZE 0
pscoast -R110/140/20/35 -JB125/20/25/45/5i -B10g5 -Dl -Glightgray -W0.25p -A250 -P > GMT_albers.ps
