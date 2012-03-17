#!/bin/sh
#	$Id$
# Testing gmtspatial intersection

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
psxy $R -Jx1.5i -P -Ba1g1WSne A.txt -W3p,red -K -X0.75i > $ps
psxy -R -J -O B.txt -W3p,blue -K >> $ps
gmtspatial A.txt B.txt -Ie | psxy -R -J -O -K -Sc0.15i -W0.25p >> $ps
# Truncate A given B
gmtspatial A.txt -TB.txt | psxy -R -J -O -K -W0.5p,green >> $ps
# Clip A and B to a smaller region
psxy -R -J -O -K -Ba1g1:.Cartesian:WSne -W0.25p,. [AB].txt -Y4i >> $ps
psxy -R -J -O -K -L -W0.5p,- << EOF >> $ps
-0.2	-0.4
1.1	-0.4
1.1	0.9
-0.2	0.9
EOF
gmtspatial A.txt -C -R-0.2/1.1/-0.4/0.9 | psxy $R -J -O -K -W3p,red >> $ps
gmtspatial B.txt -C -R-0.2/1.1/-0.4/0.9 | psxy $R -J -O -K -W3p,blue >> $ps

gmtmath -T A.txt 10 MUL = Ag.txt
gmtmath -T B.txt 10 MUL = Bg.txt
# Geographic
# Find intersections between Ag and Bg
R=-R-5/16/-5/15
psxy $R -Jm0.15i -O -Ba5g5WSne Ag.txt -W3p,red -K -Y-4i -X3.75i >> $ps
psxy -R -J -O Bg.txt -W3p,blue -K >> $ps
gmtspatial Ag.txt Bg.txt -Ie -fg | psxy -R -J -O -K -Sc0.15i -W0.25p >> $ps
# Truncate Ag given Bg
gmtspatial Ag.txt -TBg.txt -fg | psxy -R -J -O -K -W0.5p,green >> $ps
# Clip Ag and Bg to a smaller region
psxy -R -J -O -K -Ba5g5:.Geographic:WSne -W0.25p,. [AB]g.txt -Y4i >> $ps
psxy -R -J -O -K -L -W0.5p,- << EOF >> $ps
-2	-4
11	-4
11	9
-2	9
EOF
gmtspatial Ag.txt -C -R-2/11/-4/9 -fg | psxy $R -J -O -K -W3p,red >> $ps
gmtspatial Bg.txt -C -R-2/11/-4/9 -fg | psxy $R -J -O -K -W3p,blue >> $ps

psxy $R -J -O -T >> $ps

