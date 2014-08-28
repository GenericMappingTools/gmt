#!/bin/bash
#	$Id$
# Test line to polygon completion option -L<modifiers> in psxy
ps=filler.ps
cat << EOF > t.txt
1 1
2 3
3 2
4 4
EOF

gmt psxy -R0/5/0/5 -JX3i -P -K -B0 t.txt -Gred -W2p -L+yb > $ps
gmt psxy -R -J -O -K -B0 t.txt -Gred -W2p -L+yt -X3.5i >> $ps
gmt psxy -R -J -O -K -B0 t.txt -Ggreen -W2p -L+xl -X-3.5i -Y3.2i >> $ps
gmt psxy -R -J -O -K -B0 t.txt -Ggreen -W2p -L+xr -X3.5i >> $ps
gmt psxy -R -J -O -K -B0 t.txt -Gorange -W2p -L+y4 -X-3.5i -Y3.2i >> $ps
gmt psxy -R -J -O -K -B0 t.txt -Gorange -W0.5p,white -L+x4.5+p2p -X3.5i >> $ps
gmt psxy -R -J -O -T >> $ps
