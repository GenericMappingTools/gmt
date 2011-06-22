#!/bin/bash
#	$Id: GMT_polyconic.sh,v 1.5 2011-06-22 00:35:07 remko Exp $
#
. ./functions.sh

pscoast -R-180/-20/0/90 -JPoly/4i -B30g10/10g -Dc -A1000 -Glightgray -Wthinnest -P \
	> GMT_polyconic.ps
