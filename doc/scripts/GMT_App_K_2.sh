#!/bin/bash
#	$Id$
#
. ./functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 2000` -J -P -Dl -A100 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10g5WSne -K > GMT_App_K_2.ps
getrect 500 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_2.ps
