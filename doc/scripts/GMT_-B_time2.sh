#!/bin/bash
#	$Id: GMT_-B_time2.sh,v 1.9 2011-02-28 00:58:01 remko Exp $
#
. functions.sh
gmtset PLOT_DATE_FORMAT "o dd" PLOT_CLOCK_FORMAT hh:mm ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R1969-7-21T/1969-7-23T/0/1 -JX5/0.2 -Bpa6Hf1h -Bsa1KS -P -K > GMT_-B_time2.ps
psbasemap -R -J -Bpa6Hf1h -Bsa1DS -O -Y0.65i >> GMT_-B_time2.ps
