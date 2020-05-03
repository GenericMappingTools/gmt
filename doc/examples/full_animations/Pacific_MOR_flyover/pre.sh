#!/bin/bash
# Pre-script: Runs once to produce files needed for all frames
gmt begin
	gmt grdgradient @earth_relief_30s.grd -A90 -Nt2.5 -Gearth_relief_30s+2.5_int.nc
	gmt makecpt -Cgeo -H > MOR_topo.cpt
gmt end
