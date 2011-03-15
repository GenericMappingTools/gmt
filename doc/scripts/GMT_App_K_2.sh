#!/bin/bash
#	$Id: GMT_App_K_2.sh,v 1.9 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 2000` -J -P -Dl -A100 \
	-Glightgray -Wthinnest -N1/thinnest,- -B10g5WSne -K > GMT_App_K_2.ps
getrect 500 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_2.ps
