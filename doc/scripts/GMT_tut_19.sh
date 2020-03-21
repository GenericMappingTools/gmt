#!/usr/bin/env bash
gmt begin GMT_tut_19
	gmt makecpt -Cdem2 -T1000/5000
	gmt grdgradient @tut_relief.nc -Ne0.8 -A100 -fg -Gus_i.nc
	gmt grdview tut_relief.nc -JM4i -p135/35 -Qi50 -Ius_i.nc -B -JZ0.5i -C
gmt end show
