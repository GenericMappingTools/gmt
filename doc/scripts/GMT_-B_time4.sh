#!/bin/sh
#	$Id: GMT_-B_time4.sh,v 1.2 2004-04-12 19:51:12 pwessel Exp $
#
gmtset PLOT_CLOCK_FORMAT -hham
psbasemap -R0.2/0.35/0/1 -JX-6t/0.2 -Bpa1HS -Bsa15mf5m -P > GMT_-B_time4.ps
