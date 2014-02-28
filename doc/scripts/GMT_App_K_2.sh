#!/bin/bash
#	$Id$
#
gmt pscoast -Rk-2000/2000/-2000/2000 -JE130.35/-0.2/3.5i -P -Dl -A100 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10g5 -BWSne -K > GMT_App_K_2.ps
gmt psbasemap -R -J -O -Dk1000+c130.35/-0.2+pthicker >> GMT_App_K_2.ps
