#!/bin/bash
#	$Id: GMT_general_cyl.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -R-145/215/-90/90 -JY35/30/4.5i -B45g45 -Dc -A10000 -Slightgray -Wthinnest -P > \
	GMT_general_cyl.ps
