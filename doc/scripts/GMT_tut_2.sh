#!/usr/bin/env bash
gmt begin GMT_tut_2
	gmt set GMT_THEME cookbook
	gmt basemap -R1/10000/1e20/1e25 -JX9il/6il -Bxa2+l"Wavelength (m)" -Bya1pf3+l"Power (W)" -BWS
gmt end show
