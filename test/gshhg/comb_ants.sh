#!/bin/bash
#	$Id$
# Testing pscoast for plotting either of the Antarcticas in GSHHG > 2.3.0

ps=comb_ants.ps

# Plot map but skip Antarctica altogether
gmt pscoast -R-180/180/-90/-50 -JA0/-90/5i -Bag -Gperu -A+as -P -K -X0.5i -Y0.5i > $ps
echo -30 -45 NO ANT | gmt pstext -R -J -O -K -N -F+jRB+f18p >> $ps
# Plot Antarcica using icefront line as coastline and overlay grounding line
gmt pscoast -R -J -Bag -G#A5F2F3 -A+ai -O -K -X2.5i -Y4.9i >> $ps
gmt pscoast -R -J -Gbisque -A+ag -Wfaint,blue -O -K >> $ps
echo -30 -45 BOTH | gmt pstext -R -J -O -N -F+jRB+f18p >> $ps
