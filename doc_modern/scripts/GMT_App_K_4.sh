#!/bin/bash
gmt begin GMT_App_K_4 ps
gmt coast -Rk-100/100/-100/100 -JE130.35/-0.2/3.5i -Dh -A1 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B30mg10m -BWSne 
gmt basemap -Dg130.35/-0.2+w40k+jCM -F+pthicker 
gmt end
