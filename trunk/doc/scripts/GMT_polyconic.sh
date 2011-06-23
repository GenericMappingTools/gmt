#!/bin/bash
#	$Id: GMT_polyconic.sh,v 1.6 2011-06-23 01:17:06 remko Exp $
#
. ./functions.sh

pscoast -R-180/-20/0/90 -JPoly/4i -B30g10/10g10 -Dc -A1000 -Glightgray -Wthinnest -P \
	> GMT_polyconic.ps
