#!/bin/bash
#	$Id$
#
. ./functions.sh
pscoast `getbox -JE130.35/-0.2/3.5i 20` -J -P -Df \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10mg2mWSne > GMT_App_K_5.ps
