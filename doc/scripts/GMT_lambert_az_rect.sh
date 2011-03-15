#!/bin/bash
#	$Id: GMT_lambert_az_rect.sh,v 1.8 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

gmtset FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0
pscoast -R0/-40/60/-10r -JA30/-30/4.5i -B30g30/15g15 -Dl -A500 -Gp300/10 -Wthinnest -P \
	> GMT_lambert_az_rect.ps
