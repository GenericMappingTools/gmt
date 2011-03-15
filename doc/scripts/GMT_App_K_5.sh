#!/bin/bash
#	$Id: GMT_App_K_5.sh,v 1.9 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 20` -J -P -Df \
	-Glightgray -Wthinnest -N1/thinnest,- -B10mg2mWSne > GMT_App_K_5.ps
