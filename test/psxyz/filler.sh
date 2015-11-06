#!/bin/bash
#	$Id: filler.sh 13481 2014-08-28 16:58:33Z pwessel $
# Test line to polygon completion option -L<modifiers> in psxyz
ps=filler.ps
cat << EOF > t.txt
1 1 0
2 3 0
3 2 0
4 4 0
EOF

gmt psxyz -R0/5/0/5/0/1 -JX3i -JZ1i -P -K -p170/35 -B0 t.txt -Gred -W2p -L+yb > $ps
gmt psxyz -R -J -JZ -O -K -B0 -p t.txt -Gred -W2p -L+yt -X3.5i >> $ps
gmt psxyz -R -J -JZ -O -K -B0 -p t.txt -Ggreen -W2p -L+xl -X-3.5i -Y3.2i >> $ps
gmt psxyz -R -J -JZ -O -K -B0 -p t.txt -Ggreen -W2p -L+xr -X3.5i >> $ps
gmt psxyz -R -J -JZ -O -K -B0 -p t.txt -Gorange -W2p -L+y4 -X-3.5i -Y3.2i >> $ps
gmt psxyz -R -J -JZ -O -K -B0 -p t.txt -Gorange -W0.5p,white -L+x4.5+p2p -X3.5i >> $ps
gmt psxy -R0/5/0/5 -J -O -T >> $ps
