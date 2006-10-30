#!/bin/sh
#	$Id: GMT_-B_time7.sh,v 1.6 2006-10-30 19:09:11 remko Exp $
#
gmtset PLOT_DATE_FORMAT jjj TIME_INTERVAL_FRACTION 0.05
psbasemap -R2000-12-15T/2001-1-15T/0/1 -JX5/0.2 -Bpa5Df1d -Bsa1YS -P > GMT_-B_time7.ps
