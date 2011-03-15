#!/bin/bash
#	$Id: GMT_App_K_4.sh,v 1.8 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 100` -J -P -Dh -A1 \
	-Glightgray -Wthinnest -N1/thinnest,- -B30mg10mWSne -K > GMT_App_K_4.ps
getrect 20 | psxy -R -J -O -Wthicker -L -A >> GMT_App_K_4.ps
