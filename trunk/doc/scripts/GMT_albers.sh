#!/bin/bash
#	$Id: GMT_albers.sh,v 1.8 2011-06-22 21:29:04 guru Exp $
#
. ./functions.sh

gmtset MAP_GRID_CROSS_SIZE_PRIMARY 0
pscoast -R110/140/20/35 -JB125/20/25/45/5i -Bag -Dl -Ggreen -Wthinnest -A250 -P > GMT_albers.ps
