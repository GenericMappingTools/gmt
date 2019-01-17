#!/usr/bin/env bash
#
gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0 MAP_ANNOT_OBLIQUE 22 MAP_ANNOT_MIN_SPACING 0.3i
gmt pscoast -Rk-9000/9000/-9000/9000 -JE130.35/-0.2/3.5i -P -Dc \
	-A500 -Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B20g20 -BWSne -K > GMT_App_K_1.ps
gmt psbasemap -R -J -O -Dg130.35/-0.2+w4000k+jCM -F+pthicker >> GMT_App_K_1.ps
