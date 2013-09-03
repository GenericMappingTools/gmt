#!/bin/bash
# Test all the ways of labeling the bars with -D
ps=labels.ps
cat << EOF > t.txt
5
5
5
10
10
15
20
20
20
20
EOF
# Vertical bars
gmt pshistogram -R0/30/0/5 t.txt -L0.5p -Gred -B0 -P -JX2i -W5 -F -D+f12p -K -X2i -Y8i > $ps
gmt pshistogram -R t.txt -L0.5p -Gred -B0 -O -J -W5 -F -D+f12p+r -K -X2.5i >> $ps
gmt pshistogram -R t.txt -L0.5p -Gred -B0 -O -J -W5 -F -D+f12p+b -K -X-2.5i -Y-2.25i >> $ps
gmt pshistogram -R t.txt -L0.5p -Gred -B0 -O -J -W5 -F -D+f12p+r+b -K -X2.5i >> $ps
# Horizontal bars
gmt pshistogram -R t.txt -L0.5p -Gred -B0 -O -J -W5 -F -A -D+f12p -K -X-2.5i -Y-2.5i >> $ps
gmt pshistogram -R t.txt -L0.5p -Gred -B0 -O -J -W5 -F -A -D+f12p+r -K -X2.5i >> $ps
gmt pshistogram -R t.txt -L0.5p -Gred -B0 -O -J -W5 -F -A -D+f12p+b -K -X-2.5i -Y-2.25i >> $ps
gmt pshistogram -R t.txt -L0.5p -Gred -B0 -O -J -W5 -F -A -D+f12p+r+b -K -X2.5i >> $ps

gmt psxy -R -J -O -T >> $ps
