#!/bin/sh
#	$Id: GMT_stereographic_general.sh,v 1.6 2009-02-15 20:22:05 remko Exp $
#
gmtset OBLIQUE_ANNOTATION 0
pscoast -R100/-40/160/-10r -JS130/-30/4i -B30g10/15g15 -Dl -A500 -Gblack -P \
	> GMT_stereographic_general.ps
