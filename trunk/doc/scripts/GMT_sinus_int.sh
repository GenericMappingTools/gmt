#!/bin/sh
#	$Id: GMT_sinus_int.sh,v 1.1 2004-04-13 21:32:27 pwessel Exp $
#

pscoast -R200/340/-90/90 -Ji270/0.014i -Bg30/g15 -A10000 -Dc -Gblack -K -P > GMT_sinus_int.ps
pscoast -R-20/60/-90/90 -Ji20/0.014i -Bg30/g15 -Dc -A10000 -Gblack -X1.96i -O -K >> GMT_sinus_int.ps
pscoast -R60/200/-90/90 -Ji130/0.014i -Bg30/g15 -Dc -A10000 -Gblack -X1.12i -O >> GMT_sinus_int.ps
