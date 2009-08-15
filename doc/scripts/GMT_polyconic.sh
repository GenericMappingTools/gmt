#!/bin/sh
#	$Id: GMT_polyconic.sh,v 1.1 2009-08-15 20:13:05 remko Exp $
#

pscoast -R-180/-20/0/90 -JPoly/4i -B30g10/10g10 -Dc -A1000 -Glightgray -Wthinnest -P \
	> GMT_polyconic.ps
