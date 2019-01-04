#!/usr/bin/env bash
#
gmt set FORMAT_DATE_MAP "o dd" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p
gmt psbasemap -R1969-7-21T/1969-7-23T/0/1 -JX5i/0.2i -Bpa6Hf1h -Bsa1K -BS -P -K > GMT_-B_time2.ps
gmt psbasemap -R -J -Bpa6Hf1h -Bsa1D -BS -O -Y0.65i >> GMT_-B_time2.ps
