#!/bin/sh
#	$Id: GMT_-B_time2.sh,v 1.2 2004-04-12 19:51:12 pwessel Exp $
#
gmtset PLOT_CLOCK_FORMAT hh:mm
psbasemap -R2001-9-11T/2001-9-13T/0/1 -JX6t/0.2 -Bpa1KS -Bsa6Hf1h -P -K > GMT_-B_time2.ps
gmtset PLOT_DATE_FORMAT "o dd"
psbasemap -R -JX -Bpa1DS -Bsa6Hf1h -O -Y0.65i >> GMT_-B_time2.ps
