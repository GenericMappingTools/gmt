#!/bin/sh
#	$Id: GMT_App_K_4.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#
pscoast `./getbox -JE130.35/-0.2/1i -100 100 -100 100` -JE130.35/-0.2/3.5i -P -Dh -A1 -G200 \
   -W0.25p -N1/0.25tap -B30mg10mWSne -K > GMT_App_K_4.ps
./getrect -JE130.35/-0.2/1i -20 20 -20 20 | psxy -R -JE130.35/-0.2/3.5i -O -W1.5p -L -A \
   >> GMT_App_K_4.ps
