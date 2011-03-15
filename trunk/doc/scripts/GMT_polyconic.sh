#!/bin/bash
#	$Id: GMT_polyconic.sh,v 1.3 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -R-180/-20/0/90 -JPoly/4i -B30g10/10g10 -Dc -A1000 -Glightgray -Wthinnest -P \
	> GMT_polyconic.ps
