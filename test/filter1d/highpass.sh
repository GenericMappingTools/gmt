#!/bin/bash
# Testing gmt filter1d highpass and lowpass

ps=highpass.ps
gmt psxy -R0/100/-5/25 -JX6i/2.75i -P -Xc -Baf -BWSne -W0.25p raw_data.txt -K > $ps
gmt filter1d -Fm15 raw_data.txt | gmt psxy -R0/100/4/11 -J -O -K -Baf -BWSne -W0.25p,red -Y3.25i >> $ps
gmt filter1d -Fg15 raw_data.txt | gmt psxy -R -J -O -K -W0.25p,blue >> $ps
gmt filter1d -Fm15+h raw_data.txt | gmt psxy -R0/100/-13/18 -J -O -K -Bafg100 -BWSne -W0.25p,red -Y3.25i >> $ps
gmt filter1d -Fg15+h raw_data.txt | gmt psxy -R -J -O -Baf -BWSne -W0.25p,blue >> $ps
