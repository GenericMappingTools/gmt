#!/bin/bash
#	$Id: GMT_stereographic_general.sh,v 1.7 2011-02-28 00:58:00 remko Exp $
#
. functions.sh
gmtset OBLIQUE_ANNOTATION 0
pscoast -R100/-40/160/-10r -JS130/-30/4i -B30g10/15g15 -Dl -A500 -Gblack -P \
	> GMT_stereographic_general.ps
