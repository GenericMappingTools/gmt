#!/usr/bin/env bash
# Plot data reusing a column
ps=japquakes.ps

# Plot seismicity map on top, specify a 500 km scalebar kinda in the lower-right corner where there are no quakes
gmt makecpt -Cred,green,blue -T0,70,300,10000 > quakes.cpt
gmt pscoast -R130/150/35/50 -JM6i -P -Baf -BWsNe+t"Seismicity of Japan" -Sazure1 -Gburlywood -K -X1.5i -Y3.5i > $ps
gmt psxy quakes.ngdc -R -J -Sci -Cquakes.cpt -O -K -i4,3,5,6+s0.02 -hi3 >> $ps
# Along bottom plot seismicity in a depth vs longitude Cartesian plot
gmt psbasemap -R130/150/0/600 -JX6id/-2.5i -Bxaf -Byaf+l"Depth (km)" -O -K -BWSne+gazure1 -Y-2.75i >> $ps
gmt psxy -R -J -O -K -W0.5p,orange,- << EOF >> $ps
130	410
150	410
EOF
# Plot color-coded, size-challenged symbols of longitude/depth
gmt psxy quakes.ngdc -R -J -Sci -Cquakes.cpt -O -i4,5,5,6+s0.02 -hi3 >> $ps
