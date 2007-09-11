#!/bin/sh
#
#	$Id: hovmuller.sh,v 1.7 2007-09-11 22:56:12 remko Exp $

ps=hovmuller.ps
opt="--TIME_SYSTEM=other --TIME_EPOCH=2000-01-01T --TIME_UNIT=y"

echo -n "$0: Test grdimage for making Hovmuller plots:			"
awk 'BEGIN{pi=3.1415;for (y=0; y<=3; y+=0.0833333) {for (x=-180; x<=180; x+=10) {print x,y,sin(2*pi*y)*sin(pi/180*x)}}}' /dev/null > tmp.dat
xyz2grd $opt -R180w/180e/0t/3t -I10/0.0833333 tmp.dat -Gtmp.nc

makecpt -Crainbow -T-1/1/0.05 > tmp.cpt
grdimage tmp.nc -Ctmp.cpt -JX12c/12cT -B30f10/1O -Bs/1Y $opt --PLOT_DATE_FORMAT=o --TIME_FORMAT_PRIMARY=c --ANNOT_FONT_SIZE_PRIMARY=10p --ANNOT_FONT_SIZE_SECONDARY=12p -E100 > $ps

rm -f tmp.* .gmtcommands4

compare -density 100 -metric PSNR {,orig/}$ps hovmuller_diff.png > log 2>&1
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail hovmuller_diff.png log
fi
