#!/bin/sh
#	$Id: GMT_-B_time7.sh,v 1.2 2004-04-12 19:51:12 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT jjj
psbasemap -R2000-12-15T/2001-1-15T/0/1 -JX6T/0.2 -Bpa1YS -Bsa5Df1d -P > GMT_-B_time7.ps
