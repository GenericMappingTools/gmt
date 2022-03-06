#!/usr/bin/env bash
gmt begin GMT_App_K_1
	gmt set GMT_THEME cookbook
	gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0  MAP_ANNOT_MIN_SPACING 0.3i \
	  MAP_ANNOT_OBLIQUE lat_horizontal,tick_normal
	gmt coast -R-9000/9000/-9000/9000+uk -JE130.35/-0.2/3.5i -Dc \
		-A500 -Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B20g20 -BWSne
	echo 130.35 -0.2 | gmt plot -SJ-4000 -Wthicker
gmt end show
