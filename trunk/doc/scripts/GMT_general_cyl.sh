#!/bin/sh
#	$Id: GMT_general_cyl.sh,v 1.3 2004-04-13 21:32:27 pwessel Exp $
#

pscoast -R-145/215/-90/90 -JY35/30/4.5i -B45g45 -Dc -A10000 -Slightgray -W0.25p -P > \
	GMT_general_cyl.ps
