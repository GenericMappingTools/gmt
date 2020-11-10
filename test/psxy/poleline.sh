#!/usr/bin/env bash
# What Issue #1060 should look like (after fix).  In 5.3.2 we got
# fuzzy fat lines towards the pole due to excessive resampling in gmt_stat.c
ps=poleline.ps
gmt pscoast -JS21/90/18c -R0/72/70/80r -A100 -P -K -Bafg -Slightblue -W -Xc > $ps
gmt psxy -R -J -O -K line1.gmt -Wfat,navyblue@80 >> $ps
gmt psxy -J -R -O line2.gmt -Wfat,black@80 >> $ps
