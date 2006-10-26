#!/bin/sh 
#	$Id: GMT_App_K_1.sh,v 1.9 2006-10-26 18:03:07 remko Exp $
#
gmtset GRID_CROSS_SIZE_PRIMARY 0 OBLIQUE_ANNOTATION 22 ANNOT_MIN_SPACING 0.3
pscoast `./getbox.sh -JE130.35/-0.2/1i -9000 9000 -9000 9000` -JE130.35/-0.2/3.5i -P -Dc \
   -A500 -Glightgray -Wthinnest -N1/thinnest,- -B20g20WSne -K > GMT_App_K_1.ps
./getrect.sh -JE130.35/-0.2/1i -2000 2000 -2000 2000 | psxy -R -JE130.35/-0.2/3.5i -O -Wthicker -L -A \
   >> GMT_App_K_1.ps
