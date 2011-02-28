#!/bin/bash
#	$Id: GMT_perspective.sh,v 1.4 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rg -JG4/52/230/90/60/180/60/60/5i -B2g2/1g1 -Ia -Di -Glightgray -Wthinnest -P \
	--ANNOT_MIN_SPACING=0.25i > GMT_perspective.ps
