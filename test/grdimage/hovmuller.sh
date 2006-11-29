#!/bin/sh
ps=hovmuller.ps
opt="--TIME_SYSTEM=other --TIME_EPOCH=2000-01-01T --TIME_UNIT=y"

awk 'BEGIN{pi=3.1415;for (y=0; y<=3; y+=0.0833333) {for (x=-180; x<=180; x+=10) {print x,y,sin(2*pi*y)*sin(pi/180*x)}}}' /dev/null > tmp.dat
xyz2grd $opt -R180w/180e/0t/3t -I10/0.0833333 tmp.dat -Gtmp.nc

makecpt -Crainbow -T-1/1/0.05 > tmp.cpt
grdimage tmp.nc -Ctmp.cpt -JX12c/12cT -B30f10/1O -Bs/1Y $opt --PLOT_DATE_FORMAT=o --TIME_FORMAT_PRIMARY=c -E100 > $ps

rm -f tmp.* .gmtcommands4

echo -n "Comparing hovmuller_orig.ps and $ps: "
compare -density 100 -metric PSNR hovmuller_orig.ps $ps hovmuller_diff.png
