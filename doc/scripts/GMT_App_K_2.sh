#!/bin/bash
#	$Id$
#
gmt pscoast -Rk-2000/2000/-2000/2000 -JE130.35/-0.2/3.5i -P -Dl -A100 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10g5 -BWSne -K > GMT_App_K_2.ps
gmt psbasemap -R -J -O -Dg130.35/-0.2/1000k -F+pthicker >> GMT_App_K_2.ps
