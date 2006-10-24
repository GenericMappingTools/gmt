#!/bin/sh
#	$Id: GMT_general_cyl.sh,v 1.4 2006-10-24 01:53:19 remko Exp $
#

pscoast -R-145/215/-90/90 -JY35/30/4.5i -B45g45 -Dc -A10000 -Slightgray -Wthinnest -P > \
	GMT_general_cyl.ps
