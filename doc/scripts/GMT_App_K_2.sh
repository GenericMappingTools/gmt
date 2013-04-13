#!/bin/bash
#	$Id$
#
pscoast -Rk-2000/2000/-2000/2000 -JE130.35/-0.2/3.5i -P -Dl -A100 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10g5WSne -K > GMT_App_K_2.ps
getrect 500 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_2.ps
