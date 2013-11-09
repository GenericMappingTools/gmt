#!/bin/bash
#	$Id$
# Testing pscoast for plotting either of the Antarcticas in GSHHG > 2.3.0

ps=two_ants.ps

# Plot Antarcica using icefront as coastline
gmt pscoast -R-180/180/-90/-50 -JA0/-90/5i -Bag -G#A5F2F3 -A+ai -P -K -X0.5i -Y0.5i > $ps
echo -30 -45 ICE | gmt pstext -R -J -O -K -N -F+jRB+f18p >> $ps
# Plot Antarcica using grounding line as coastline
gmt pscoast -R -J -Bag -Gbisque -A+ag -O -K -X2.5i -Y4.9i >> $ps
echo -30 -45 GROUND | gmt pstext -R -J -O -N -F+jRB+f18p >> $ps
