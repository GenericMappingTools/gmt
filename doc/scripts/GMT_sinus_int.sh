#!/bin/bash
#	$Id: GMT_sinus_int.sh,v 1.5 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -R200/340/-90/90 -Ji0.014i -Bg30/g15 -A10000 -Dc -Gblack -K -P > GMT_sinus_int.ps
pscoast -R-20/60/-90/90 -Ji0.014i -Bg30/g15 -Dc -A10000 -Gblack -X1.96i -O -K >> GMT_sinus_int.ps
pscoast -R60/200/-90/90 -Ji0.014i -Bg30/g15 -Dc -A10000 -Gblack -X1.12i -O >> GMT_sinus_int.ps
