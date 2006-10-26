#!/bin/sh
#	$Id: GMT_App_K_3.sh,v 1.8 2006-10-26 18:03:07 remko Exp $
#
pscoast `./getbox.sh -JE130.35/-0.2/1i -500 500 -500 500` -JE130.35/-0.2/3.5i -P -Di -A20 -Glightgray \
   -Wthinnest -N1/thinnest,- -B2g1WSne -K > GMT_App_K_3.ps
echo 133 2 | psxy -R -J -O -K -Sc1.4i -Gwhite >> GMT_App_K_3.ps
psbasemap -R -J -O -K -Tm133/2/1i::+45/10/5 --HEADER_FONT_SIZE=12p --TICK_LENGTH=0.05i \
   --ANNOT_FONT_SIZE_SECONDARY=8p >> GMT_App_K_3.ps
./getrect.sh -JE130.35/-0.2/1i -100 100 -100 100 | psxy -R -JE130.35/-0.2/3.5i -O -Wthicker -L -A \
   >> GMT_App_K_3.ps
