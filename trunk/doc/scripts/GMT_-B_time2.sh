#!/bin/sh
#	$Id: GMT_-B_time2.sh,v 1.1 2001-09-25 01:09:26 pwessel Exp $
#
gmtset PLOT_CLOCK_FORMAT hh:mm
psbasemap -R2001-9-11T/2001-9-13T/0/1 -JX6t/0.2 -Ba6Hf1hIf1KS -P -K > GMT_-B_time2.ps
gmtset PLOT_DATE_FORMAT "o dd"
psbasemap -R -JX -Ba6Hf1hIf1DS -O -Y0.65i >> GMT_-B_time2.ps
