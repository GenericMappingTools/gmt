#!/bin/bash
#	$Id$
#
. ./functions.sh
gmtset FORMAT_DATE_MAP o TIME_WEEK_START Sunday FORMAT_CLOCK_MAP=-hham FORMAT_TIME_PRIMARY_MAP full
psbasemap -R2001-9-24T/2001-9-29T/T07:0/T15:0 -JX4i/-2i -Ba1Kf1kg1d/a1Hg1hWsNe+glightyellow \
	-P > GMT_linear_cal.ps
