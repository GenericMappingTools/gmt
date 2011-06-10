#!/bin/bash
#	$Id: GMT_sinus_int.sh,v 1.6 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -R200/340/-90/90 -Ji0.014i -Bg30/g15 -A10000 -Dc -Gdarkred -Sazure -K -P > GMT_sinus_int.ps
pscoast -R-20/60/-90/90 -J -B -Dc -A10000 -Gdarkgreen -Sazure -X1.96i -O -K >> GMT_sinus_int.ps
pscoast -R60/200/-90/90 -J -B -Dc -A10000 -Gdarkblue -Sazure -X1.12i -O >> GMT_sinus_int.ps
