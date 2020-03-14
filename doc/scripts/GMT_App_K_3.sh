#!/usr/bin/env bash
gmt begin GMT_App_K_3
	gmt coast -R-500/500/-500/500+uk -JE130.35/-0.2/3.5i -Di -A20 -Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B2g1 -BWSne
	echo 133 2 | gmt plot -Sc1.4i -Gwhite
	gmt basemap -Tmg133/2+w1i+t45/10/5+jCM --FONT_TITLE=12p --MAP_TICK_LENGTH_PRIMARY=0.05i --FONT_ANNOT_SECONDARY=8p
	echo 130.35 -0.2 | gmt plot -SJ-200 -Wthicker
gmt end show
