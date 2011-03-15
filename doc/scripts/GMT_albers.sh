#!/bin/bash
#	$Id: GMT_albers.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

gmtset MAP_GRID_CROSS_SIZE_PRIMARY 0
pscoast -R110/140/20/35 -JB125/20/25/45/5i -B10g5 -Dl -Ggreen -Wthinnest -A250 -P > GMT_albers.ps
