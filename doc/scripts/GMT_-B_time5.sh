#!/usr/bin/env bash
gmt begin GMT_-B_time5
	gmt set GMT_THEME cookbook
	gmt set FORMAT_DATE_MAP u FORMAT_TIME_PRIMARY_MAP Character FORMAT_TIME_SECONDARY_MAP full FONT_ANNOT_PRIMARY +9p
	gmt basemap -R1969-7-21T/1969-8-9T/0/1 -JX12c/0.5c -Bpxa1K -Bsxa1U -BS
	gmt set FORMAT_DATE_MAP o TIME_WEEK_START Sunday FORMAT_TIME_SECONDARY_MAP Character
	gmt basemap -Bpxa3Kf1k -Bsxa1r -BS -Y1.6c
gmt end show
