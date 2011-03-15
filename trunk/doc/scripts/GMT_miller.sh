#!/bin/bash
#	$Id: GMT_miller.sh,v 1.7 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -R-90/270/-80/90 -Jj1:400000000 -B45g45/30g30 -Dc -A10000 -Glightgray -Wthinnest -P \
	> GMT_miller.ps
