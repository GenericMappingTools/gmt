#!/bin/bash
#	$Id: GMT_perspective.sh,v 1.7 2011-06-22 00:35:07 remko Exp $
#
. ./functions.sh

pscoast -Rg -JG4/52/230/90/60/180/60/60/5i -B2g/1g -Ia -Di -Glightbrown -Wthinnest -P \
	-Slightblue --MAP_ANNOT_MIN_SPACING=0.25i > GMT_perspective.ps
