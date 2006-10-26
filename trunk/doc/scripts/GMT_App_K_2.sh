#!/bin/sh
#	$Id: GMT_App_K_2.sh,v 1.5 2006-10-26 18:03:07 remko Exp $
#
pscoast `./getbox.sh -JE130.35/-0.2/1i -2000 2000 -2000 2000` -JE130.35/-0.2/3.5i -P -Dl -A100 \
   -Glightgray -Wthinnest -N1/thinnest,- -B10g5WSne -K > GMT_App_K_2.ps
./getrect.sh -JE130.35/-0.2/1i -500 500 -500 500 | psxy -R -JE130.35/-0.2/3.5i -O -Wthicker -L -A \
   >> GMT_App_K_2.ps
