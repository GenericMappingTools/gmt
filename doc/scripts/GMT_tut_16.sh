#!/usr/bin/env bash
gmt begin GMT_tut_16
	gmt set GMT_THEME cookbook
	gmt makecpt -Crainbow -T1000/5000
	gmt grdcut @earth_relief_30s -R-108/-103/35/40 -Gtut_relief.nc
	gmt grdgradient tut_relief.nc -Ne0.8 -A100 -fg -Gus_i.nc
	gmt grdimage @earth_relief_30s -R-108/-103/35/40 -Ius_i.nc -JM6i -B -BWSnE
	gmt colorbar -DJTC -I0.4 -Bxa -By+lm
gmt end show
