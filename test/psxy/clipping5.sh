#!/bin/bash
#       $Id$
#
# Check ellipse clipping and filling/outline
# Note: This will pass the test since the original is the same as the output
# The issues have been posted to gmt-dev forum
ps=clipping5.ps

cat << EOF > t.txt
170	48	30	4000	2000
240	-68	0	4500	4500
90	0	-30	4000	5000
EOF

gmt psxy -R0/360/-60/60 -JM6i -P -K -Baf -BWSne -Xc -SE -Gred -W5p,blue t.txt > $ps
echo "60	-68	0	4500	4500" | gmt psxy -R -J -O -K -SE -W5p,blue >> $ps
echo "300	68	0	4500	4500" | gmt psxy -R -J -O -K -SE -W5p,blue --PS_LINE_CAP=square >> $ps
echo "10 50 D" | gmt pstext -R -J -O -K -F+jCM+f18p >> $ps
cat << EOF > p.txt
> clipped at bottom
100	-45
130	-40
140	-47
150	-54
130	-65
90	-60
> clipped at left/right
30	20
60	23
80	35
70	38
40	35
20	33
5	29
-15	24
-25	20
-40	10
-33	5
-15	15
-5	18
5	19
EOF
gmt psxy -R -J -O -K -Baf -BWSne -Gorange -W5p,blue p.txt -Y3i >> $ps
echo "10 50 C" | gmt pstext -R -J -O -K -F+jCM+f18p >> $ps
cat << EOF > s.txt
0	1	c
1	0	s
2	1	t
1	2	g
1	1	a
EOF
gmt psxy -R0/2/0/2 -JX2.75i -O -K -Baf -BWSne -S1i -Gyellow -W5p,blue s.txt -Y3i >> $ps
echo "0.2 1.8 A" | gmt pstext -R -J -O -K -F+jCM+f18p >> $ps
cat << EOF > v.txt
-0.9	-0.9	93	135
-0.9	-0.9	60	270
-0.9	-0.9	30	270
-0.9	-0.9	-3	160
EOF
gmt psxy -R-1/1/-1/1 -JM2.75i -O -K -Baf -BwSnE -S=0.5i+e -Gcyan -W5p,brown v.txt -X3.25i >> $ps
echo "-0.8 0.8 B" | gmt pstext -R -J -O -K -F+jCM+f18p >> $ps
gmt psxy -R -J -O -T >> $ps
