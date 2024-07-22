#!/usr/bin/env bash
gmt blockmedian -R245/255/20/30 -I5m -V @tut_ship.xyz > ship_5m.xyz
gmt surface ship_5m.xyz -R245/255/20/30 -I5m -Gship.nc
gmt begin GMT_tut_13
	gmt mask -R245/255/20/30 -I5m ship_5m.xyz -JM6i -B
	gmt grdcontour ship.nc -C250 -A1000
	gmt mask -C
gmt end show
