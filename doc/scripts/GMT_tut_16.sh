#!/usr/bin/env bash
gmt begin GMT_tut_16
	gmt makecpt -Crainbow -T1000/5000
	gmt grdgradient @tut_relief.nc -Ne0.8 -A100 -fg -Gus_i.nc
	gmt grdimage tut_relief.nc -Ius_i.nc -JM6i -B -BWSnE
	gmt colorbar -DJTC -I0.4 -Bxa -By+lm
gmt end show
