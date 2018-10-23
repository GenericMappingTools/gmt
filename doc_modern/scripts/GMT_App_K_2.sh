#!/bin/bash
gmt begin GMT_App_K_2 ps
gmt coast -Rk-2000/2000/-2000/2000 -JE130.35/-0.2/3.5i -Dl -A100 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10g5 -BWSne 
gmt basemap -Dg130.35/-0.2+w1000k+jCM -F+pthicker
gmt end
