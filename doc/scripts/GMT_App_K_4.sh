#!/bin/sh
#	$Id: GMT_App_K_4.sh,v 1.3 2006-10-24 01:53:19 remko Exp $
#
pscoast `./getbox -JE130.35/-0.2/1i -100 100 -100 100` -JE130.35/-0.2/3.5i -P -Dh -A1 -Glightgray \
   -Wthinnest -N1/thinnest,- -B30mg10mWSne -K > GMT_App_K_4.ps
./getrect -JE130.35/-0.2/1i -20 20 -20 20 | psxy -R -JE130.35/-0.2/3.5i -O -Wthicker -L -A \
   >> GMT_App_K_4.ps
