#!/bin/bash
#	$Id: GMT_App_K_2.sh,v 1.10 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 2000` -J -P -Dl -A100 \
	-Glightgray -Wthinnest -N1/thinnest,- -B10g5WSne -K > GMT_App_K_2.ps
getrect 500 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_2.ps
