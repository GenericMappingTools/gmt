#!/usr/bin/env bash
gmt begin GMT_tut_19
	gmt set GMT_THEME cookbook
	gmt makecpt -Cdem2 -T1000/5000
	gmt grdview @earth_relief_30s -R-108/-103/35/40 -JM4i -p135/35 -Qi50 -I+a100+ne0.8 -B -JZ0.5i
gmt end show
