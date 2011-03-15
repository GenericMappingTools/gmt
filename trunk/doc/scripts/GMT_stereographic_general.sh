#!/bin/bash
#	$Id: GMT_stereographic_general.sh,v 1.8 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
gmtset MAP_ANNOT_OBLIQUE 0
pscoast -R100/-42/160/-8r -JS130/-30/4i -B30g10/15g15 -Dl -A500 -Ggreen -P \
	-Slightblue -Wthinnest > GMT_stereographic_general.ps
