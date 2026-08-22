#!/usr/bin/env bash
# Make sure the adding arrows at end of lines with two points work for geo and Cart
ps=endarrows.ps
cat << EOF > t.txt
-50 40
50 -40
EOF
gmt psxy -R-60/60/-50/50 -JX4i -W0.25p -Baf -P t.txt -K -Xc > $ps
gmt psxy -R -J -W1p+o0.5i/0.25i+v0.25i+gred -O t.txt -K >> $ps
gmt psxy -R -JM4i -W0.25p -Baf -O -K t.txt -Y5i >> $ps
gmt psxy -R -J -W1p+o1000k/1000n+v0.25i+gred -O t.txt  >> $ps
