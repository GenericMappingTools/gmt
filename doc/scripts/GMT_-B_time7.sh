#!/bin/sh
#	$Id: GMT_-B_time7.sh,v 1.4 2004-05-07 22:07:08 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT jjj TIME_INTERVAL_FRACTION 0.05
psbasemap -R2000-12-15T/2001-1-15T/0/1 -JX6T/0.2 -Bpa5Df1d -Bsa1YS -P > GMT_-B_time7.ps
