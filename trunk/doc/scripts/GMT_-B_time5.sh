#!/bin/sh
#	$Id: GMT_-B_time5.sh,v 1.3 2004-04-12 21:41:47 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT u TIME_FORMAT_PRIMARY full
psbasemap -R2001-9-11T/2001-9-29T/0/1 -JX6t/0.2 -Bpa1US -Bsa1K -P -K > GMT_-B_time5.ps
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday TIME_FORMAT_PRIMARY Char
psbasemap -R2001-9-8T/2001-9-29T/0/1 -JX6t/0.2 -Bpa1rS -Bsa3Kf1k -O -Y0.65i >> GMT_-B_time5.ps
