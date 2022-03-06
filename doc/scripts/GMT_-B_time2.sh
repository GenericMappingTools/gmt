#!/usr/bin/env bash
gmt begin GMT_-B_time2
	gmt set GMT_THEME cookbook
  gmt set FORMAT_DATE_MAP "o dd" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p
  gmt basemap -R1969-7-21T/1969-7-23T/0/1 -JX12c/0.5c -Bpxa6Hf1h -Bsxa1K -BS
  gmt basemap -Bpxa6Hf1h -Bsxa1D -BS -Y1.6c
gmt end show
