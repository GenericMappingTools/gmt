#!/usr/bin/env bash
gmt begin GMT_-B_time3
	gmt set FORMAT_DATE_MAP o FORMAT_TIME_PRIMARY_MAP Character FONT_ANNOT_PRIMARY +9p
	gmt basemap -R1997T/1999T/0/1 -JX12c/0.5c -Bpa3Of1o -Bsa1Y -BS
gmt end show
