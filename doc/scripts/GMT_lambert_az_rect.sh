#!/bin/bash
#	$Id: GMT_lambert_az_rect.sh,v 1.10 2011-06-22 00:35:06 remko Exp $
#
. ./functions.sh

gmtset FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0
pscoast -R0/-40/60/-10r -JA30/-30/4.5i -B30g/15g -Dl -A500 -Gp300/10 -Wthinnest -P \
	> GMT_lambert_az_rect.ps
