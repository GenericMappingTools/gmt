#!/usr/bin/env bash
#
# Another test for gmt project in generating small and great circles
ps=circles.ps
# Small circle 30 degrees colat
gmt project -C202/24.5 -E240.5/31 -G5/30 -Q | gmt psxy -R190/250/20/35 -JM6i -P -K -Baf -Xc > $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred << EOF >> $ps
202    24.5
240.5    31
EOF
gmt pstext -R -J -O -K -F+cTL+jTL+f14p+t"Colatitude: 30 degrees" -Dj0.1i >> $ps
# Small circle 45 degrees colat
gmt project -C202/24.5 -E240.5/31 -G5/45 -Q | gmt psxy -R -J -O -K -Baf -Y2.5i >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred << EOF >> $ps
202    24.5
240.5    31
EOF
gmt pstext -R -J -O -K -F+cTL+jTL+f14p+t"Colatitude: 45 degrees" -Dj0.1i >> $ps
# Small circle 60 degrees colat
gmt project -C202/24.5 -E240.5/31 -G5/45 -Q | gmt psxy -R -J -O -K -Baf -Y2.5i >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred << EOF >> $ps
202    24.5
240.5    31
EOF
gmt pstext -R -J -O -K -F+cTL+jTL+f14p+t"Colatitude: 60 degrees" -Dj0.1i >> $ps
# Great circle
gmt project -C202/24.5 -E240.5/31 -G5 -Q | gmt psxy -R -J -O -K -Baf -Y2.5i >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred << EOF >> $ps
202    24.5
240.5    31
EOF
gmt pstext -R -J -O -F+cTL+jTL+f14p+t"Colatitude: 90 degrees" -Dj0.1i >> $ps
