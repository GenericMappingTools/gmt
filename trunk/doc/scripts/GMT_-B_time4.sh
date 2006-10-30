#!/bin/sh
#	$Id: GMT_-B_time4.sh,v 1.4 2006-10-30 02:59:36 remko Exp $
#
gmtset PLOT_CLOCK_FORMAT -hham
psbasemap -R0.2t/0.35t/0/1 -JX-6/0.2 -Bpa15mf5m -Bsa1HS -P > GMT_-B_time4.ps
