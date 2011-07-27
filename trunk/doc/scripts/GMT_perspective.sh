#!/bin/bash
#	$Id$
#
. ./functions.sh

pscoast -Rg -JG4/52/230/90/60/180/60/60/5i -B2g2/1g1 -Ia -Di -Glightbrown -Wthinnest -P \
	-Slightblue --MAP_ANNOT_MIN_SPACING=0.25i > GMT_perspective.ps
