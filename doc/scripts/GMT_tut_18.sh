#!/usr/bin/env bash
gmt begin GMT_tut_18
	gmt grdview @earth_relief_05m -R-66/-60/30/35 -JM5i -JZ2i -p135/30 -B
gmt end show
