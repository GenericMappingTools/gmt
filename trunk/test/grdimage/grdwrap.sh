#!/bin/sh
#
#	$Id: grdwrap.sh,v 1.3 2007-10-24 01:11:41 remko Exp $

ps=grdwrap.ps
grdmath -Rg -I1 X SIND Y COSD MUL = t.grd=ns/0.0001
makecpt -Cpolar -T-1/1/0.01 > t.cpt

grdimage t.grd -Ct.cpt -JQ-147/6i -Ct.cpt -B30 -K > $ps
grdimage t.grd -Ct.cpt -JQ-147.533/6i -Ct.cpt -B30 -O -Y3.75i >> $ps
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
