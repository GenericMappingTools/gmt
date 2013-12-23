#!/bin/bash
#	$Id$
# Testing gmt gmtspatial intersection

ps=intersect.ps

cat << EOF > A.txt
0	0
0.6	-0.3
1.3	0
1	0.9
0.8	0.2
0	1
-0.3	0.7
0	0
EOF
cat << EOF > B.txt
0.3	0.3
1	0.6
1.5	0.3
1.4	1.1
0.7	1.2
0.1	1
0.3	0.3
EOF

# Cartesian
# Find intersections between A and B
R=-R-0.5/1.6/-0.5/1.5
gmt psxy $R -Jx1.5i -P -Ba1g1 -BWSne A.txt -W3p,red -K -X0.75i > $ps
gmt psxy $R -J -O B.txt -W3p,blue -K >> $ps
gmt gmtspatial A.txt B.txt -Ie | gmt psxy $R -Jx1.5i -O -K -Sc0.15i -W0.25p >> $ps
# Truncate A given B
gmt gmtspatial A.txt -TB.txt | gmt psxy $R -Jx1.5i -O -K -W0.5p,green >> $ps
# Clip A and B to a smaller region
gmt psxy $R -J -O -K -Ba1g1 -BWSne+tCartesian -W0.25p,. [AB].txt -Y4i >> $ps
gmt psxy $R -J -O -K -L -W0.5p,- << EOF >> $ps
-0.2	-0.4
1.1	-0.4
1.1	0.9
-0.2	0.9
EOF
gmt gmtspatial A.txt -C -R-0.2/1.1/-0.4/0.9 | gmt psxy $R -Jx1.5i -O -K -W3p,red >> $ps
gmt gmtspatial B.txt -C -R-0.2/1.1/-0.4/0.9 | gmt psxy $R -Jx1.5i -O -K -W3p,blue >> $ps

gmt gmtmath -T A.txt 10 MUL = Ag.txt
gmt gmtmath -T B.txt 10 MUL = Bg.txt
# Geographic
# Find intersections between Ag and Bg
R=-R-5/16/-5/15
gmt psxy $R -Jm0.15i -O -Ba5g5 -BWSne Ag.txt -W3p,red -K -Y-4i -X3.75i >> $ps
gmt psxy $R -J -O Bg.txt -W3p,blue -K >> $ps
gmt gmtspatial Ag.txt Bg.txt -Ie -fg | gmt psxy $R -Jm0.15i -O -K -Sc0.15i -W0.25p >> $ps
# Truncate Ag given Bg
gmt gmtspatial Ag.txt -TBg.txt -fg | gmt psxy $R -Jm0.15i -O -K -W0.5p,green >> $ps
# Clip Ag and Bg to a smaller region
gmt psxy $R -J -O -K -Ba5g5 -BWSne+tGeographic -W0.25p,. [AB]g.txt -Y4i >> $ps
gmt psxy $R -J -O -K -L -W0.5p,- << EOF >> $ps
-2	-4
11	-4
11	9
-2	9
EOF
gmt gmtspatial Ag.txt -C -R-2/11/-4/9 -fg | gmt psxy $R -Jm0.15i -O -K -W3p,red >> $ps
gmt gmtspatial Bg.txt -C -R-2/11/-4/9 -fg | gmt psxy $R -Jm0.15i -O -K -W3p,blue >> $ps

gmt psxy $R -J -O -T >> $ps

