#!/usr/bin/env bash
#
# Test gmt psxy for proper handling of -a [OGR].  We read an OGR file
# with depth and magnitude among the aspatial values and we wish to
# use that as input cols 2 and 3, then scale col 3 (mag, which is a
# log10 measure) to get symbol sizes.

ps=quakes.ps

gmt makecpt -Crainbow -T0/300 > t.cpt
gmt psxy @quakes.gmt -R15/25/15/25 -JM6i -B5 -Sci -Ct.cpt -P -K -Wthin -a2=depth,3=magnitude -i0:2,3+s0.05 -Yc -Xc > $ps
gmt pstext @quakes.gmt -R -J -O -K -a2=name -F+jCT -Dj0/0.2i >> $ps
gmt psscale -Ct.cpt -Dx3i/-0.5i+w6i/0.1i+h+jTC -O -Bxa50+l"Epicenter Depth" -By+lkm >> $ps
