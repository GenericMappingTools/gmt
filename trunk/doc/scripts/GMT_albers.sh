#!/bin/bash
#	$Id: GMT_albers.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

gmtset GRID_CROSS_SIZE_PRIMARY 0
pscoast -R110/140/20/35 -JB125/20/25/45/5i -B10g5 -Dl -Glightgray -Wthinnest -A250 -P > GMT_albers.ps
