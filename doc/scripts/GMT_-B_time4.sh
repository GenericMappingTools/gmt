#!/bin/sh
#	$Id: GMT_-B_time4.sh,v 1.3 2004-04-13 17:37:39 pwessel Exp $
#
gmtset PLOT_CLOCK_FORMAT -hham
psbasemap -R0.2/0.35/0/1 -JX-6t/0.2 -Bpa15mf5m -Bsa1HS -P > GMT_-B_time4.ps
