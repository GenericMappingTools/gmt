#!/bin/sh
#	$Id: GMT_App_K_2.sh,v 1.7 2007-06-05 15:44:51 remko Exp $
#
pscoast `./getbox.sh -JE130.35/-0.2/3.5i 2000` -J -P -Dl -A100 \
	-Glightgray -Wthinnest -N1/thinnest,- -B10g5WSne -K > GMT_App_K_2.ps
./getrect.sh 500 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_2.ps
