#!/bin/bash
#	$Id: GMT_miller.sh,v 1.11 2011-06-23 01:17:06 remko Exp $
#
. ./functions.sh

pscoast -R-90/270/-80/90 -Jj1:400000000 -B45g45/30g30 -Dc -A10000 -Gkhaki -Wthinnest -P \
	-Sazure > GMT_miller.ps
