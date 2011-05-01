#!/bin/bash
#	$Id: GMT_App_K_1.sh,v 1.14 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
gmtset MAP_GRID_CROSS_SIZE_PRIMARY 0 MAP_ANNOT_OBLIQUE 22 MAP_ANNOT_MIN_SPACING 0.3i
pscoast `getbox -JE130.35/-0.2/3.5i 9000` -J -P -Dc \
	-A500 -Glightgray -Wthinnest -N1/thinnest,- -B20g20WSne -K > GMT_App_K_1.ps
getrect 2000 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_1.ps
