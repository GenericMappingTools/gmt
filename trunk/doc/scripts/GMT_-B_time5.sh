#!/bin/bash
#	$Id: GMT_-B_time5.sh,v 1.9 2011-02-28 00:58:03 remko Exp $
#
. functions.sh
gmtset PLOT_DATE_FORMAT u TIME_FORMAT_PRIMARY Character TIME_FORMAT_SECONDARY full \
	ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R1969-7-21T/1969-8-9T/0/1 -JX5/0.2 -Bpa1K -Bsa1US -P -K > GMT_-B_time5.ps
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday TIME_FORMAT_SECONDARY Chararacter
psbasemap -R -J -Bpa3Kf1k -Bsa1rS -O -Y0.65i >> GMT_-B_time5.ps
