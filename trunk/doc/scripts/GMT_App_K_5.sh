#!/bin/bash
#	$Id: GMT_App_K_5.sh,v 1.10 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 20` -J -P -Df \
	-Glightgray -Wthinnest -N1/thinnest,- -B10mg2mWSne > GMT_App_K_5.ps
