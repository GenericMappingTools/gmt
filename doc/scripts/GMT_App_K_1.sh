#!/bin/sh 
#	$Id: GMT_App_K_1.sh,v 1.7 2004-07-14 00:46:17 pwessel Exp $
#
gmtset GRID_CROSS_SIZE_PRIMARY 0 OBLIQUE_ANNOTATION 22 ANNOT_MIN_SPACING 0.3
pscoast `./getbox -JE130.35/-0.2/1i -9000 9000 -9000 9000` -JE130.35/-0.2/3.5i -P -Dc \
   -A500 -Glightgray -W0.25p -N1/0.25tap -B20g20WSne -K > GMT_App_K_1.ps
./getrect -JE130.35/-0.2/1i -2000 2000 -2000 2000 | psxy -R -JE130.35/-0.2/3.5i -O -W1.5p -L -A \
   >> GMT_App_K_1.ps
