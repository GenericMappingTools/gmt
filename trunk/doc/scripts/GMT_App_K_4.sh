#!/bin/bash
#	$Id$
#
pscoast -Rk-100/100/-100/100 -JE130.35/-0.2/3.5i -P -Dh -A1 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B30mg10mWSne -K > GMT_App_K_4.ps
(echo -20 -20; echo -20 20; echo 20 20; echo 20 -20) | psxy -R -J -O -Wthicker -L -A -fpk >> GMT_App_K_4.ps