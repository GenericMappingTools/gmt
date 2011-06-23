#!/bin/bash
#	$Id: GMT_perspective.sh,v 1.8 2011-06-23 01:17:06 remko Exp $
#
. ./functions.sh

pscoast -Rg -JG4/52/230/90/60/180/60/60/5i -B2g2/1g1 -Ia -Di -Glightbrown -Wthinnest -P \
	-Slightblue --MAP_ANNOT_MIN_SPACING=0.25i > GMT_perspective.ps
