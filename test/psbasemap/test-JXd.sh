#!/bin/bash
#
#	$Id$

basemap="gmt psbasemap --FORMAT_GEO_MAP=dddF --FONT_LABEL=16p --MAP_FRAME_AXES=WeSn"

plot1 () {
   $basemap -R -JX$1 -Bx20f10g20+lLongitude -By20f10g20+lLatitude -O -K $2
   annot $1
}

annot () {
   gmt psxy -R -J -O -K -W2p,red -Gyellow <<%
-40 0
40 0
40 -40
-40 -40
%
   gmt pstext -R -J -F+f20p,Helvetica-Bold+jBC -O -K <<< "0 2 -JX$1"
}

ps=test-JXd.ps

gmt psxy -T -R-60/60/-60/60 -JX8c/8c -K -X4c -Y13c > $ps
plot1 8c/8c >> $ps
plot1 8cd/8c -X12c >> $ps
plot1 8c/8cd -Y-11c >> $ps
plot1 8cd/8cd -X-12c >> $ps
gmt psxy -T -R -J -O >> $ps

