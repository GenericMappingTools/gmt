#!/bin/sh
#	$Id: GMT_App_K_4.sh,v 1.6 2007-06-05 15:44:51 remko Exp $
#
pscoast `./getbox.sh -JE130.35/-0.2/3.5i 100` -J -P -Dh -A1 \
	-Glightgray -Wthinnest -N1/thinnest,- -B30mg10mWSne -K > GMT_App_K_4.ps
./getrect.sh 20 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_4.ps
