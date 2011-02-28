#!/bin/bash
#	$Id: GMT_App_K_5.sh,v 1.8 2011-02-28 00:58:03 remko Exp $
#
. functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 20` -J -P -Df \
	-Glightgray -Wthinnest -N1/thinnest,- -B10mg2mWSne > GMT_App_K_5.ps
