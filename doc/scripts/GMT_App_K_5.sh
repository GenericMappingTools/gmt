#!/bin/sh
#	$Id: GMT_App_K_5.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#
pscoast `./getbox -JE130.35/-0.2/1i -20 20 -20 20` -JE130.35/-0.2/3.5i -P -Df -Glilghtgray -W0.25p \
   -N1/0.25tap -B10mg2mWSne > GMT_App_K_5.ps
