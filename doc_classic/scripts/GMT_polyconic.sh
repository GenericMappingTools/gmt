#!/bin/bash
#	$Id$
#
gmt pscoast -R-180/-20/0/90 -JPoly/4i -Bx30g10 -By10g10 -Dc -A1000 -Glightgray -Wthinnest -P \
	> GMT_polyconic.ps
