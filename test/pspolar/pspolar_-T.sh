#!/usr/bin/env bash
#
# Test the new syntax of pspolar -T
#
ps=pspolar_-T.ps
gmt psbasemap -R-2/2/-2/2 -JX10c -Baf -K > $ps
for just in BL BC BR ML MC MR TL TC TR; do
    echo "$just 30 45 +" | gmt pspolar -J -R -D0/0 -M5c -Sc0.5c -T+f10p,red+j$just -K -O >> $ps
done
gmt psxy -R -J -T -O >> $ps
