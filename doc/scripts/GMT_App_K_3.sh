#!/bin/sh
#	$Id: GMT_App_K_3.sh,v 1.5 2004-07-13 18:47:09 pwessel Exp $
#
pscoast `./getbox -JE130.35/-0.2/1i -500 500 -500 500` -JE130.35/-0.2/3.5i -P -Di -A20 -Glightgray \
   -W0.25p -N1/0.25tap -B2g1WSne -K > GMT_App_K_3.ps
echo 133 2 | psxy -R -J -O -K -Sc1.4i -Gwhite >> GMT_App_K_3.ps
psbasemap -R -J -O -K -Tm133/2/1i+45/10/5:: --HEADER_FONT_SIZE=12p --TICK_LENGTH=0.05i \
   --ANNOT_FONT_SIZE_SECONDARY=8p >> GMT_App_K_3.ps
./getrect -JE130.35/-0.2/1i -100 100 -100 100 | psxy -R -JE130.35/-0.2/3.5i -O -W1.5p -L -A \
   >> GMT_App_K_3.ps
