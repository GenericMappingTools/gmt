#!/bin/sh
#	$Id: GMT_App_K_2.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#
pscoast `./getbox -JE130.35/-0.2/1i -2000 2000 -2000 2000` -JE130.35/-0.2/3.5i -P -Dl -A100 -G200 \
   -W0.25p -N1/0.25tap -B10g5WSne -K > GMT_App_K_2.ps
./getrect -JE130.35/-0.2/1i -500 500 -500 500 | psxy -R -JE130.35/-0.2/3.5i -O -W1.5p -L -A \
   >> GMT_App_K_2.ps
