#!/bin/sh
#	$Id: GMT_linear_cal.sh,v 1.1 2001-09-25 01:38:42 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday PLOT_CLOCK_FORMAT -hham
psbasemap -R2001-9-24T/2001-9-29T/T07:0/T15:0 -JX4T/-2T -Bf1kif1Kg1d/a1Hg1hWsNe -P > GMT_linear_cal.ps
