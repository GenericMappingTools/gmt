#!/usr/bin/env bash
gmt begin GMT_tut_16 ps
	gmt makecpt -Crainbow -T1000/5000
	gmt grdgradient @tut_relief.nc -Ne0.8 -A100 -fg -Gus_i.nc
	gmt grdimage tut_relief.nc -Ius_i.nc -JM6i -Ba -BWSnE -C
	gmt colorbar -DJTC -Rtut_relief.nc -C -I0.4 -Bxa -By+lm 
gmt end
