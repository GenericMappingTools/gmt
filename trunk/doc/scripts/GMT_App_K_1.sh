#!/bin/bash
#	$Id: GMT_App_K_1.sh,v 1.16 2011-06-22 00:35:06 remko Exp $
#
. ./functions.sh
gmtset MAP_GRID_CROSS_SIZE_PRIMARY 0 MAP_ANNOT_OBLIQUE 22 MAP_ANNOT_MIN_SPACING 0.3i
pscoast `getbox -JE130.35/-0.2/3.5i 9000` -J -P -Dc \
	-A500 -Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B20gWSne -K > GMT_App_K_1.ps
getrect 2000 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_1.ps
