#!/bin/sh
#	$Id: GMT_-B_time7.sh,v 1.1 2001-09-25 01:09:26 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT jjj
psbasemap -R2000-12-15T/2001-1-15T/0/1 -JX6T/0.2 -Bi5Df1dI1YS -P > GMT_-B_time7.ps
