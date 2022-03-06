#!/usr/bin/env bash
gmt begin GMT_-B_time1
	gmt set GMT_THEME cookbook
	gmt set FORMAT_DATE_MAP=-o FONT_ANNOT_PRIMARY +9p
	gmt basemap -R2000-4-1T/2000-5-25T/0/1 -JX12c/0.5c -Bpxa7Rf1d -Bsxa1O -BS
gmt end show
