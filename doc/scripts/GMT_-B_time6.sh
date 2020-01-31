#!/usr/bin/env bash
gmt begin GMT_-B_time6
	gmt set FORMAT_DATE_MAP "o yy" FORMAT_TIME_PRIMARY_MAP Abbreviated
	gmt basemap -R1996T/1996-6T/0/1 -JX12c/0.5c -Ba1Of1d -BS
gmt end show
