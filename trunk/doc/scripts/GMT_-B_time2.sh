#!/bin/bash
#	$Id: GMT_-B_time2.sh,v 1.10 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
gmtset FORMAT_DATE_MAP "o dd" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p
psbasemap -R1969-7-21T/1969-7-23T/0/1 -JX5i/0.2i -Bpa6Hf1h -Bsa1KS -P -K > GMT_-B_time2.ps
psbasemap -R -J -Bpa6Hf1h -Bsa1DS -O -Y0.65i >> GMT_-B_time2.ps
