#!/bin/sh
#	$Id: GMT_stereographic_general.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#
gmtset DEGREE_FORMAT 1 OBLIQUE_ANOTATION 0
pscoast -R100/-40/160/-10r -JS130/-30/4i -B30g10/15g15 -Dl -A500 -G0 -P \
    > GMT_stereographic_general.ps
