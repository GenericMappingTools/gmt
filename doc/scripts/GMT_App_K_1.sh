#!/bin/bash 
#	$Id: GMT_App_K_1.sh,v 1.12 2011-02-28 00:58:00 remko Exp $
#
. functions.sh
gmtset GRID_CROSS_SIZE_PRIMARY 0 OBLIQUE_ANNOTATION 22 ANNOT_MIN_SPACING 0.3
pscoast `getbox -JE130.35/-0.2/3.5i 9000` -J -P -Dc \
	-A500 -Glightgray -Wthinnest -N1/thinnest,- -B20g20WSne -K > GMT_App_K_1.ps
getrect 2000 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_1.ps
