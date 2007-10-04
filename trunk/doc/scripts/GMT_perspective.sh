#!/bin/sh
#	$Id: GMT_perspective.sh,v 1.2 2007-10-04 15:49:37 remko Exp $
#

pscoast -Rg -JG4/52/230/90/60/180/60/60/5i -B2g2/1g1 -Ia -Di -Glightgray -Wthinnest -P \
	> GMT_perspective.ps
