#!/bin/bash
#	$Id: GMT_miller.sh,v 1.6 2011-02-28 00:58:00 remko Exp $
#
. functions.sh

pscoast -R-90/270/-80/90 -Jj1:400000000 -B45g45/30g30 -Dc -A10000 -Glightgray -Wthinnest -P \
	> GMT_miller.ps
