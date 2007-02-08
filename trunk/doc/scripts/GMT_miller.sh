#!/bin/sh
#	$Id: GMT_miller.sh,v 1.4 2007-02-08 21:46:28 remko Exp $
#

pscoast -R-90/270/-80/90 -Jj90/1:400000000 -B45g45/30g30 -Dc -A10000 -Glightgray -Wthinnest -P \
	> GMT_miller.ps
