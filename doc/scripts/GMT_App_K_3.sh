#!/bin/bash
#	$Id: GMT_App_K_3.sh,v 1.14 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 500` -J -P -Di -A20 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B2g1WSne -K > GMT_App_K_3.ps
echo 133 2 | psxy -R -J -O -K -Sc1.4i -Gwhite >> GMT_App_K_3.ps
psbasemap -R -J -O -K -Tm133/2/1i::+45/10/5 --FONT_TITLE=12p --MAP_TICK_LENGTH=0.05i \
	--FONT_ANNOT_SECONDARY=8p >> GMT_App_K_3.ps
getrect 100 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_3.ps
