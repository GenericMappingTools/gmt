#!/bin/sh
#	$Id: GMT_App_K_5.sh,v 1.4 2006-10-24 01:53:19 remko Exp $
#
pscoast `./getbox -JE130.35/-0.2/1i -20 20 -20 20` -JE130.35/-0.2/3.5i -P -Df -Glightgray -Wthinnest \
   -N1/thinnest,- -B10mg2mWSne > GMT_App_K_5.ps
