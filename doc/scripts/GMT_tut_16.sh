#!/usr/bin/env bash
gmt begin GMT_tut_16
	gmt set GMT_THEME cookbook
	gmt makecpt -Crainbow -T1000/5000
	gmt grdimage @earth_relief_30s -R-108/-103/35/40 -I+a100+ne0.8 -JM6i -B -BWSnE
	gmt colorbar -DJTC -I0.4 -Bxa -By+lm
gmt end show
