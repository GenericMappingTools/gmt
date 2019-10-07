#!/usr/bin/env bash
# Make sure filter1d spatial filtering works
ps=smooth_track.ps
# km
gmt psxy -R0/30/0/15 -JM6i -P track.txt -Wfaint,blue -Baf -BWSne -K -Y0.75i > $ps
gmt filter1d track.txt -Ngk+a -E -Fg200 | gmt psxy -R -J -O -K -W0.25p,red >> $ps
#nm
gmt psxy -R -J track.txt -Wfaint,blue -Baf -BWsne -O -K -Y3.3i >> $ps
gmt filter1d track.txt -Ngn -E -Fg200 | gmt psxy -R -J -O -K -W0.25p,red >> $ps

#Cartesian
gmt psxy -R -JX6i/3i track.txt -Wfaint,blue -Baf -BWsne -O -K -Y3.3i >> $ps
gmt filter1d track.txt -Nc -E -Fg2 | gmt psxy -R -J -O -W0.25p,red >> $ps
