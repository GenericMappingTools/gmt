#!/bin/sh
#	$Id: GMT_-B_time4.sh,v 1.1 2001-09-25 01:09:26 pwessel Exp $
#
gmtset PLOT_CLOCK_FORMAT -hham
psbasemap -R0.2/0.35/0/1 -JX-6t/0.2 -Ba15mA1Hf5mS -P >> GMT_-B_time4.ps
