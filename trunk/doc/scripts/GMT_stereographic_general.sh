#!/bin/bash
#	$Id: GMT_stereographic_general.sh,v 1.10 2011-06-22 00:35:07 remko Exp $
#
. ./functions.sh
gmtset MAP_ANNOT_OBLIQUE 0
pscoast -R100/-42/160/-8r -JS130/-30/4i -B30g10/15g -Dl -A500 -Ggreen -P \
	-Slightblue -Wthinnest > GMT_stereographic_general.ps
