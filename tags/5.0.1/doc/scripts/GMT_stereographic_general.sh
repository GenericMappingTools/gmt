#!/bin/bash
#	$Id$
#
. ./functions.sh
gmtset MAP_ANNOT_OBLIQUE 0
pscoast -R100/-42/160/-8r -JS130/-30/4i -Bag -Dl -A500 -Ggreen -P \
	-Slightblue -Wthinnest > GMT_stereographic_general.ps
