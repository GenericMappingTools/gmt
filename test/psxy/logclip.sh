#!/usr/bin/env bash
#
# Check clipping of symbols with error bars
ps=logclip.ps
cat << EOF > data.txt
1 120   0.5 40
2  50   0.5 30
3  20   0.5 10
4   5   0.5  3
5   2   0.5  2
6   1.2 0.5  1
7   0.8 0.5  1
8   0.3 0.5  1
EOF

gmt psxy -R0/10/1e0/1e2 -Jx0.20i/1.0il -Xc -Y2i -BneWS -Bxa5f -Bya1f3p data.txt -Wthick -Exy0i/thick -Sc.0001i -P -K > $ps
gmt psxy -R -J -Y4i -BneWS -Bxa5f -Bya1f3p data.txt -Wthick -Exy0i/thick -Sc.0001i -N -O >> $ps
rm -f data.txt
