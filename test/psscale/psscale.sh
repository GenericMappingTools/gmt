#!/usr/bin/env bash
#

ps=psscale.ps

gmt set FONT_ANNOT_PRIMARY 10p FONT_LABEL 14p PROJ_LENGTH_UNIT cm

gmt makecpt -T-6/6/1 -Cseis -D > tmp.cpt

scale="gmt psscale -Ctmp.cpt -Bx+lRange -By+lm"

$scale -K    -Dx00/04+w8/0.5+e+jML -Y2 > $ps
$scale -O -K -Dx04/04+w8/0.5+e+jML+ma >> $ps
$scale -O -K -Dx07/04+w8/0.5+e+jML+ml >> $ps
$scale -O -K -Dx11/04+w8/0.5+e+jML+m >> $ps
$scale -O -K -Dx18/00+w8/0.5+e+jTC+h+m >> $ps
$scale -O -K -Dx18/03+w8/0.5+e+jTC+h+ml >> $ps
$scale -O -K -Dx18/05+w8/0.5+e+jTC+h+ma >> $ps
$scale -O -K -Dx18/08+w8/0.5+e+jTC+h >> $ps

$scale -O -K -Dx00/04+w8/0.5+e+jML+mc -Y9 >> $ps
$scale -O -K -Dx04/04+w8/0.5+e+jML+mac >> $ps
$scale -O -K -Dx07/04+w8/0.5+e+jML+mlc >> $ps
$scale -O -K -Dx11/04+w8/0.5+e+jML+malc >> $ps
$scale -O -K -Dx18/00+w8/0.5+e+jTC+h+malc >> $ps
$scale -O -K -Dx18/03+w8/0.5+e+jTC+h+ml >> $ps
$scale -O -K -Dx18/05+w8/0.5+e+jTC+h+ma >> $ps
$scale -O    -Dx18/08+w8/0.5+e+jTC+h+mc >> $ps
