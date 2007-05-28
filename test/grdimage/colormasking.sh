#!/bin/sh
#
#	$Id: colormasking.sh,v 1.5 2007-05-28 23:25:36 pwessel Exp $

ps=colormasking.ps
echo -n "$0: Test grdimage for use of color masking:		"
#grdmath -R0/3/0/3 -I1 X Y DIV = t.grd
xyz2grd -R-0.5/2.5/-0.5/2.5 -I1 -F -Gt.grd <<%
0 0 0.0
0 1 0.2
0 2 0.4
1 0 1.0
1 1 NaN
1 2 1.4
2 0 2.0
2 1 2.2
2 2 2.4
%
makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
grdimage t.grd -JX1c -Ct.cpt -K -P -X8c -Y8c > $ps
grdimage t.grd -JX3c -Ct.cpt -K -O -X-1c -Y-1c -Q >> $ps
grdimage t.grd -JX9c -Ct.cpt -K -O -X-3c -Y-3c -Q >> $ps
psxy -R -J -Gp50/10:FwhiteB- -O >> $ps <<%
1 0
2 0
2 1
1 1
%
rm -f t.grd t.cpt .gmtcommands4

compare -density 100 -metric PSNR colormasking_orig.ps $ps colormasking_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
else
        echo "[PASS]"
        rm -f fail colormasking_diff.png log
fi
