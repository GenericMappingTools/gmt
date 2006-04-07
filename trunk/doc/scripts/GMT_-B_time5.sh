#!/bin/sh
#	$Id: GMT_-B_time5.sh,v 1.5 2006-04-07 22:07:46 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT u TIME_FORMAT_SECONDARY full
psbasemap -R1969-7-21T/1969-8-9T/0/1 -JX6t/0.2 -Bpa1K -Bsa1US -P -K > GMT_-B_time5.ps
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday TIME_FORMAT_PRIMARY Char TIME_FORMAT_SECONDARY Char
psbasemap -R1969-7-18T/1969-8-9T/0/1 -JX6t/0.2 -Bpa3Kf1k -Bsa1rS -O -Y0.65i >> GMT_-B_time5.ps
