#!/usr/bin/env bash
gmt begin GMT_tut_19
	gmt set GMT_THEME cookbook
	gmt makecpt -Cdem2 -T1000/5000
	gmt grdcut @earth_relief_30s -R-108/-103/35/40 -Gtut_relief.nc
	gmt grdgradient tut_relief.nc -Ne0.8 -A100 -fg -Gus_i.nc
	gmt grdview tut_relief.nc -JM4i -p135/35 -Qi50 -Ius_i.nc -B -JZ0.5i
gmt end show
