#!/bin/sh
#	$Id: GMT_stereographic_general.sh,v 1.4 2004-04-10 17:19:14 pwessel Exp $
#
gmtset PLOT_DEGREE_FORMAT ddd:mm:ss OBLIQUE_ANNOTATION 0
pscoast -R100/-40/160/-10r -JS130/-30/4i -B30g10/15g15 -Dl -A500 -Gblack -P \
    > GMT_stereographic_general.ps
