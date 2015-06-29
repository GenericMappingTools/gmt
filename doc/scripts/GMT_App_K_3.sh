#!/bin/bash
#	$Id$
#
gmt pscoast -Rk-500/500/-500/500 -JE130.35/-0.2/3.5i -P -Di -A20 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B2g1 -BWSne -K > GMT_App_K_3.ps
echo 133 2 | gmt psxy -R -J -O -K -Sc1.4i -Gwhite >> GMT_App_K_3.ps
gmt psbasemap -R -J -O -K -Tm133/2+w1i+t45/10/5 --FONT_TITLE=12p --MAP_TICK_LENGTH_PRIMARY=0.05i \
	--FONT_ANNOT_SECONDARY=8p >> GMT_App_K_3.ps
gmt psbasemap -R -J -O -Dg130.35/-0.2+w200k -F+pthicker >> GMT_App_K_3.ps
