#!/bin/bash
#	$Id$
#

ps=psscale.ps

gmt gmtset FONT_ANNOT_PRIMARY 10p FONT_LABEL 14p PROJ_LENGTH_UNIT cm

gmt makecpt -T-6/6/1 -Cseis -D > tmp.cpt

plot () {
$scale -D00/04/8/0.5 -K $1
$scale -D04/04/8/0.5 -Aa -O -K
$scale -D07/04/8/0.5 -Al -O -K
$scale -D11/04/8/0.5 -A -O -K
$scale -D18/00/8/0.5h -A -O -K
$scale -D18/03/8/0.5h -Al -O -K
$scale -D18/05/8/0.5h -Aa -O -K
$scale -D18/08/8/0.5h -O $2
}

scale="gmt psscale -E -Ctmp.cpt -Bx+lRange -By+lm"
plot -Y2 -K > $ps
scale="gmt psscale -E -Ctmp.cpt -Ac -Bx1+lRange -By+lm"
plot "-Y9 -O" >> $ps

