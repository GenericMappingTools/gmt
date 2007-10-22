#!/bin/sh
#
#	$Id: grdwrap.sh,v 1.2 2007-10-22 18:27:26 remko Exp $

ps=grdwrap.ps
grdmath -R0/360/-90/90 -I1 X Y MUL = t.grd
makecpt -Crainbow -T-33000/33000/1000 > t.cpt

grdimage t.grd -Ct.cpt -JQ-147/6i -Ct.cpt -B30 -K > $ps
grdimage t.grd -Ct.cpt -JQ-147.533/6i -Ct.cpt -B30 -O -K -Y3.75i >> $ps
echo -n "$0: Test grdimage for wrapping of global:			"

rm -f t.grd t.cpt .gmtcommands4

compare -density 100 -metric PSNR {,orig/}$ps grdwrap_diff.png > log 2>&1
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail grdwrap_diff.png log
fi
