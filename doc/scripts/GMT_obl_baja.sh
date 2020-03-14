#!/usr/bin/env bash
# Oblique Mercator map for Baja California with oblique Equator along y-axis
gmt begin GMT_obl_baja
	gmt set MAP_ANNOT_OBLIQUE 14
	gmt coast -R122W/35N/107W/22N+r -JOa120W/25N/-30/6c+v -Gsienna -Ba5g5 -B+f -N1/1p -EUS+gburlywood -Smintcream -TdjBL+w0.5i+l
gmt end show
