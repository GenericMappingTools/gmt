#!/bin/bash
#	$Id$
#
. ./functions.sh

pscoast -R-180/-20/0/90 -JPoly/4i -B30g10/10g10 -Dc -A1000 -Glightgray -Wthinnest -P \
	> GMT_polyconic.ps
