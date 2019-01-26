#!/usr/bin/env bash
gmt set FORMAT_DATE_MAP jjj TIME_INTERVAL_FRACTION 0.05 FONT_ANNOT_PRIMARY +9p
gmt basemap -R2000-12-15T/2001-1-15T/0/1 -JX5i/0.2i -Bpa5Df1d -Bsa1Y -BS -ps GMT_-B_time7
