#!/bin/bash
#	$Id: GMT_App_K_4.sh,v 1.9 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 100` -J -P -Dh -A1 \
	-Glightgray -Wthinnest -N1/thinnest,- -B30mg10mWSne -K > GMT_App_K_4.ps
getrect 20 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_4.ps
