#!/bin/sh
#	$Id: GMT_stereographic_general.sh,v 1.2 2001-09-14 18:30:17 pwessel Exp $
#
gmtset PLOT_DEGREE_FORMAT ddd:mm:ss OBLIQUE_ANOTATION 0
pscoast -R100/-40/160/-10r -JS130/-30/4i -B30g10/15g15 -Dl -A500 -G0 -P \
    > GMT_stereographic_general.ps
