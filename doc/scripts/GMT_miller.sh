#!/bin/sh
#	$Id: GMT_miller.sh,v 1.5 2008-04-28 17:45:43 remko Exp $
#

pscoast -R-90/270/-80/90 -Jj1:400000000 -B45g45/30g30 -Dc -A10000 -Glightgray -Wthinnest -P \
	> GMT_miller.ps
