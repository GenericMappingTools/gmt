#!/usr/bin/env bash
gmt begin GMT_linear_cal
	gmt set FORMAT_DATE_MAP o TIME_WEEK_START Sunday FORMAT_CLOCK_MAP=-hham FORMAT_TIME_PRIMARY_MAP full
	gmt basemap -R2001-9-24T/2001-9-29T/T07:0/T15:0 -JX10c/-5c -Bxa1Kf1kg1d -Bya1Hg1h -BWsNe+glightyellow
gmt end show
