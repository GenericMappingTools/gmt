#!/bin/sh
#	$Id: GMT_App_K_5.sh,v 1.7 2007-06-05 15:44:51 remko Exp $
#
pscoast `./getbox.sh -JE130.35/-0.2/3.5i 20` -J -P -Df \
	-Glightgray -Wthinnest -N1/thinnest,- -B10mg2mWSne > GMT_App_K_5.ps
