#!/bin/sh 
#	$Id: GMT_App_K_1.sh,v 1.3 2003-04-14 21:56:56 pwessel Exp $
#
gmtset GRID_CROSS_SIZE 0 OBLIQUE_ANOTATION 0 ANNOT_MIN_SPACING 0.25
pscoast `./getbox -JE130.35/-0.2/1i -9000 9000 -9000 9000` -JE130.35/-0.2/3.5i -P -Dc \
   -A500 -G200 -W0.25p -N1/0.25tap -B20g20WSne -K \
   | egrep -v '\(80\\312|\(100\\312|\(120\\312|\(140\\312|\(160\\312|\(180\\312' > GMT_App_K_1.ps
./getrect -JE130.35/-0.2/1i -2000 2000 -2000 2000 | psxy -R -JE130.35/-0.2/3.5i -O -W1.5p -L -A \
   >> GMT_App_K_1.ps
