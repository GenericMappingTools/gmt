#!/bin/sh

ps=test-JXd-1.ps

plot1 () {
   psbasemap -R -JX$1 -B20f10g20:Longitude:/20f10g20:Latitude: --PLOT_DEGREE_FORMAT=dddF -O -K $2
   annot $1
}

annot () {
   psxy -R -J -O -K -W2p/red -Gyellow <<%
-40 0
40 0
40 -40
-40 -40
%
   pstext -R -J -O -K <<%
0 2 20 0 1 BC -JX$1
%
}

psxy /dev/null -R-60/60/-60/60 -JX8c/8c -K -X4c -Y13c > $ps
plot1 8c/8c >> $ps
plot1 8cd/8c -X12c >> $ps
plot1 8c/8cd -Y-11c >> $ps
plot1 8cd/8cd -X-12c >> $ps
psxy /dev/null -R -J -O >> $ps

ps=test-JXd-2.ps
psxy /dev/null -R-60/60/-60/60 -JX8c/8c -K -X4c -Y13c > $ps
plot1 8c/8c >> $ps
psbasemap -R -JX8cd/8cd -B20f10g20/Sn --PLOT_DEGREE_FORMAT=dddF --BASEMAP_TYPE=plain -X12c -O -K >> $ps
psbasemap -R -JX8c/8c -B:Longitude:/20f10g20:Latitude: --PLOT_DEGREE_FORMAT=dddF -O -K >> $ps
annot 8cd/8c >> $ps
psbasemap -R -JX8cd/8cd -B/20f10g20We --PLOT_DEGREE_FORMAT=dddF -Y-11c -O -K >> $ps
psbasemap -R -JX8c/8c -B20f10g20:Longitude:/:Latitude: --PLOT_DEGREE_FORMAT=dddF -O -K >> $ps
annot 8c/8cd >> $ps
plot1 8cd/8cd -X-12c >> $ps
psxy /dev/null -R -J -O >> $ps
