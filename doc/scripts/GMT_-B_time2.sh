#!/bin/sh
#	$Id: GMT_-B_time2.sh,v 1.3 2004-04-13 17:37:39 pwessel Exp $
#
gmtset PLOT_CLOCK_FORMAT hh:mm
psbasemap -R2001-9-11T/2001-9-13T/0/1 -JX6t/0.2 -Bpa6Hf1h -Bsa1KS -P -K > GMT_-B_time2.ps
gmtset PLOT_DATE_FORMAT "o dd"
psbasemap -R -JX -Bpa6Hf1h -Bsa1DS -O -Y0.65i >> GMT_-B_time2.ps
