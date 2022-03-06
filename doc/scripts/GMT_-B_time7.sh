#!/usr/bin/env bash
gmt begin GMT_-B_time7
	gmt set GMT_THEME cookbook
	gmt set FORMAT_DATE_MAP jjj TIME_INTERVAL_FRACTION 0.05 FONT_ANNOT_PRIMARY +9p
	gmt basemap -R2000-12-15T/2001-1-15T/0/1 -JX12c/0.5c -Bpxa5Df1d -Bsxa1Y -BS
gmt end show
