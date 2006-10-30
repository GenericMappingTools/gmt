#!/bin/sh
#	$Id: GMT_-B_time2.sh,v 1.7 2006-10-30 19:09:11 remko Exp $
#
gmtset PLOT_CLOCK_FORMAT hh:mm
psbasemap -R1969-7-21T/1969-7-23T/0/1 -JX5/0.2 -Bpa6Hf1h -Bsa1KS -P -K > GMT_-B_time2.ps
gmtset PLOT_DATE_FORMAT "o dd"
psbasemap -R -J -Bpa6Hf1h -Bsa1DS -O -Y0.65i >> GMT_-B_time2.ps
