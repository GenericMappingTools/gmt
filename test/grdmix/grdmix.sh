#!/usr/bin/env bash
#
# Testing gmt grdmix capabilities

ps=grdmix.ps
# Testing grdmix
gmt grdmath -R0/5/0/5 -I1 -r X = x.grd
gmt grdmath -R0/5/0/5 -I1 -r Y = y.grd
echo "0 red 10 red" > t.cpt
gmt grdimage x.grd -Ct.cpt -Jx1 -Ared.png
echo "0 blue 10 blue" > t.cpt
gmt grdimage y.grd -Ct.cpt -Jx1 -Ablue.png
gmt grdmath -R0/5/0/5 -I1 -r Y X MUL DUP UPPER DIV = w.grd
echo "0 black 1 white" > t.cpt
gmt grdimage w.grd -Ct.cpt -Jx1 -Aalpha.png
gdal_translate red.png red.ppm
gdal_translate blue.png blue.ppm
gdal_translate alpha.png alpha.ppm
gmt grdmix red.ppm blue.ppm -Aalpha.ppm -Gmix.ppm
gdal_translate mix.ppm mix.png
gmt psimage mix.png -Dx0/0+w5i -P -B0 -K -X1.75i > $ps
gmt psimage red.png -Dx0/0+w1.5i -O -K -B0 -Y5.5i >> $ps
gmt psimage blue.png -Dx0/0+w1.5i -O -K -B0 -X1.75i >> $ps
gmt psimage alpha.png -Dx0/0+w1.5i -O -B0 -X1.75i >> $ps
