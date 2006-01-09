#!/bin/sh

ps=test-JXd.ps

plot () {
   psbasemap -R -JX$1 -B20f10g20:Longitude:/20f10g20:Latitude: --PLOT_DEGREE_FORMAT=dddF -O -K $2
   pstext -R -J -O -K <<%
0 22 20 0 1 BC -JX$1
%
}

psxy /dev/null -R-60/60/-60/60 -JX8c/8c -K -X4c -Y13c > $ps
plot 8c/8c >> $ps
plot 8cd/8c "-X12c" >> $ps
plot 8c/8cd "-Y-11c" >> $ps
plot 8cd/8cd "-X-12c" >> $ps
psxy /dev/null -R -J -O >> $ps
