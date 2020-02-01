#!/usr/bin/env bash
#

ps=seis.ps

gmt set FONT_ANNOT_PRIMARY 10p PROJ_LENGTH_UNIT cm

gmt makecpt -T-6/6/1 -Cseis -D > tmp.cpt
gmt psscale -Ctmp.cpt -Dx00/04+w8/0.5+jML -K           > $ps
gmt psscale -Ctmp.cpt -Dx00/13+w8/0.5+jML -O -K -I -N100 >> $ps
gmt makecpt -T-6/6/1 -Cseis -D | grep -v ^0 | sed 's/^1/0/' > tmp.cpt
gmt psscale -Ctmp.cpt -Dx04/04+w8/0.5+jML -O -K       >> $ps
gmt psscale -Ctmp.cpt -Dx04/13+w8/0.5+jML -O -K -I -N100 >> $ps
gmt psscale -Ctmp.cpt -Dx08/04+w8/0.5+jML -O -K -L    >> $ps
gmt psscale -Ctmp.cpt -Dx08/13+w8/0.5+jML -O -K -L -I -N100 >> $ps
gmt psscale -Ctmp.cpt -Dx12/04+w8/0.5+jML -O -K -L0.1    >> $ps
gmt psscale -Ctmp.cpt -Dx12/13+w8/0.5+jML -O -K -L0.1 -I -N100 >> $ps
gmt makecpt -T-6/6/1 -Cseis -D -Z > tmp.cpt
gmt psscale -Ctmp.cpt -Dx16/04+w8/0.5+jML -O -K       >> $ps
gmt psscale -Ctmp.cpt -Dx16/13+w8/0.5+jML -O -K -I -N100 >> $ps
gmt makecpt -T-6/6/1 -Cseis -D -Z | grep -v ^0 | sed 's/^1/0/' > tmp.cpt
gmt psscale -Ctmp.cpt -Dx20/04+w8/0.5+jML -O -K       >> $ps
gmt psscale -Ctmp.cpt -Dx20/13+w8/0.5+jML -O    -I -N100 >> $ps

