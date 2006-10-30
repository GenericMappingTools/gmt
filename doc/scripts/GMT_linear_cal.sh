#!/bin/sh
#	$Id: GMT_linear_cal.sh,v 1.3 2006-10-30 02:59:36 remko Exp $
#
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday PLOT_CLOCK_FORMAT -hham TIME_FORMAT_PRIMARY full
psbasemap -R2001-9-24T/2001-9-29T/T07:0/T15:0 -JX4i/-2i -Ba1Kf1kg1d/a1Hg1hWsNe -P > GMT_linear_cal.ps
