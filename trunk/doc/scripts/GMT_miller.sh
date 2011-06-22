#!/bin/bash
#	$Id: GMT_miller.sh,v 1.10 2011-06-22 00:35:06 remko Exp $
#
. ./functions.sh

pscoast -R-90/270/-80/90 -Jj1:400000000 -B45g/30g -Dc -A10000 -Gkhaki -Wthinnest -P \
	-Sazure > GMT_miller.ps
