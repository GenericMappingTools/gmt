#!/bin/bash
#	$Id: GMT_linear_cal.sh,v 1.4 2011-02-28 00:58:03 remko Exp $
#
. functions.sh
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday PLOT_CLOCK_FORMAT -hham TIME_FORMAT_PRIMARY full
psbasemap -R2001-9-24T/2001-9-29T/T07:0/T15:0 -JX4i/-2i -Ba1Kf1kg1d/a1Hg1hWsNe -P > GMT_linear_cal.ps
