#!/usr/bin/env bash
gmt begin GMT_tut_11
	gmt set GMT_THEME cookbook
	gmt grdcontour @earth_relief_05m -R-66/-60/30/35 -JM6i -C250 -A1000 -B
gmt end show
