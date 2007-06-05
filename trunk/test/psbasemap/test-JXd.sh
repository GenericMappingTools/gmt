#!/bin/sh
#
#	$Id: test-JXd.sh,v 1.8 2007-06-05 14:02:36 remko Exp $

psbasemap="psbasemap --PLOT_DEGREE_FORMAT=dddF --LABEL_FONT_SIZE=16p --BASEMAP_AXES=WeSn"
psxy=psxy

plot1 () {
   $psbasemap -R -JX$1 -B20f10g20:Longitude:/20f10g20:Latitude: -O -K $2
   annot $1
}

annot () {
   $psxy -R -J -O -K -W2p/red -Gyellow <<%
-40 0
40 0
40 -40
-40 -40
%
   pstext -R -J -O -K <<%
0 2 20 0 1 BC -JX$1
%
}

echo -n "$0: Test various specifications of -Jx w/wo trailing d:	"
ps=test-JXd.ps
psxy /dev/null -R-60/60/-60/60 -JX8c/8c -K -X4c -Y13c > $ps
plot1 8c/8c >> $ps
plot1 8cd/8c -X12c >> $ps
plot1 8c/8cd -Y-11c >> $ps
plot1 8cd/8cd -X-12c >> $ps
psxy /dev/null -R -J -O >> $ps

rm -f .gmtcommands4

compare -density 100 -metric PSNR {,orig/}$ps test-JXd_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail test-JXd_diff.png log
fi
