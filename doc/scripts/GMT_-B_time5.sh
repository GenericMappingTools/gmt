#!/bin/sh
#	$Id: GMT_-B_time5.sh,v 1.4 2004-04-13 17:37:39 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT u TIME_FORMAT_SECONDARY full
psbasemap -R2001-9-11T/2001-9-29T/0/1 -JX6t/0.2 -Bpa1K -Bsa1US -P -K > GMT_-B_time5.ps
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday TIME_FORMAT_PRIMARY Char
psbasemap -R2001-9-8T/2001-9-29T/0/1 -JX6t/0.2 -Bpa3Kf1k -Bsa1rS -O -Y0.65i >> GMT_-B_time5.ps
