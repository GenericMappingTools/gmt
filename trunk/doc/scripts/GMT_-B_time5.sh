#!/bin/sh
#	$Id: GMT_-B_time5.sh,v 1.2 2004-04-12 19:51:12 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT u TIME_ANNOT_FORMAT full
psbasemap -R2001-9-11T/2001-9-29T/0/1 -JX6t/0.2 -Bpa1US -Bsa1K -P -K > GMT_-B_time5.ps
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday TIME_ANNOT_FORMAT Char
psbasemap -R2001-9-8T/2001-9-29T/0/1 -JX6t/0.2 -Bpa1rS -Bsa3Kf1k -O -Y0.65i >> GMT_-B_time5.ps
