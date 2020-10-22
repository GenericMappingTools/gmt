#!/usr/bin/env bash
# Testing psrose with uncentered bins for sectors and roses
ps=rose_uncentered.ps
cat << EOF > data.txt
20 5.4 5.4 2.4 1.2
40 2.2 2.2 0.8 0.7
60 1.4 1.4 0.7 0.7
80 1.1 1.1 0.6 0.6
100 1.2 1.2 0.7 0.7
120 2.6 2.2 1.2 0.7
140 8.9 7.6 4.5 0.9
160 10.6 9.3 5.4 1.1
180 8.2 6.2 4.2 1.1
200 4.9 4.1 2.5 1.5
220 4 3.7 2.2 1.5
240 3 3 1.7 1.5
260 2.2 2.2 1.3 1.2
280 2.1 2.1 1.4 1.3
300 2.5 2.5 1.4 1.2
320 5.5 5.3 2.5 1.2
340 17.3 15 8.8 1.4
360 25 14.2 7.5 1.3
EOF
# Sector diagram
gmt psrose data.txt -i1,0 -A20 -JX3i -P -R0/25/0/360 -Bxa10g10 -K -Bya10g10 -B+t"Sector Diagram" -W1p -Gorange > $ps
gmt psrose data.txt -i2,0 -A20 -JX3i -R -O -K -W1p -Ggreen >> $ps
gmt psrose data.txt -i3,0 -A20 -JX3i -R -O -K -W1p -Gblue >> $ps
gmt psrose data.txt -i4,0 -A20 -JX3i -R -O -K -W1p -Gwhite >> $ps
# Windrose diagram
gmt psrose data.txt -i1,0 -JX3i -R -Bxa10g10 -O -K -Bya10g10 -B+t"Windrose Diagram" -W4p,orange -X3.5i -Y2.75i >> $ps
gmt psrose data.txt -i2,0 -JX3i -R -O -K -W4p,green >> $ps
gmt psrose data.txt -i3,0 -JX3i -R -O -K -W4p,blue >> $ps
gmt psrose data.txt -i4,0 -JX3i -R -O -K -W4p,white >> $ps
# Rose diagram
gmt psrose data.txt -i1,0 -A20r -JX3i -R -Bxa10g10 -O -K -Bya10g10 -B+t"Rose Diagram" -W1p -Gorange -X-3.5i -Y2.75i >> $ps
gmt psrose data.txt -i2,0 -A20r -JX3i -R -O -K -W1p -Ggreen >> $ps
gmt psrose data.txt -i3,0 -A20r -JX3i -R -O -K -W1p -Gblue >> $ps
gmt psrose data.txt -i4,0 -A20r -JX3i -R -O -K -W1p -Gwhite >> $ps
echo "4 2 Bins are not centered" | gmt pstext -R0/6/0/3 -Jx1i -O -F+f18p+jLM -N >> $ps
