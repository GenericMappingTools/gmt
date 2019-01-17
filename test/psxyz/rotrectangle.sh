#!/usr/bin/env bash
#
# Test that psxyz properly plots rotatable rectangles -Sj and -SJ

ps=rotrectangle.ps

# Bottom case tests -SJ with azimuths and dimensions in km
cat << EOF > tt.rects.d
-65 15 0 90 500 200
-70 20 0 0 500 200
-80 25 0 70 500 300
EOF
gmt psxyz -JZ2 -p135/30 -R270/20/305/25/0/3r -JOc280/25.5/22/69/5i -P -B10g5 -BWSne -SJ tt.rects.d -Gred -W0.25p,green -K > $ps
gmt psxyz -JZ -p135/30 -R -J -O -K -Sc0.05i tt.rects.d -Gblack >> $ps
# Middle case tests -Sj with angles and dimensions in inches (hence trailing i)
cat << EOF > tt.rects.d
-75 15 0 0 1 0.5
-70 20 0 30 1 0.5
-80 25 0 90 0.5 0.2
EOF
gmt psxyz -JZ -p135/30 -R -J -O -K -B10g5 -BWSne -Sji tt.rects.d -Gblue -W0.25p,green -Y3i >> $ps
gmt psxyz -JZ -p135/30 -R -J -O -K -Sc0.05i tt.rects.d -Gblack >> $ps
# Top case is just Cartesian case where we pass angle and dimensions in -R units
cat << EOF > tt.rects.d
0 0 0 30 5 2
10 10 0 70 7 1
20 0 0 90 6 3
EOF
gmt psxyz -JZ -p135/30 -R-10/25/-5/15/0/4 -Jx0.15i -O -K -B10g5 -BWSne -SJ tt.rects.d -Gbrown -W0.25p,green -Y3i >> $ps
gmt psxyz -JZ -p135/30 -R -J -O -Sc0.05i tt.rects.d -Gblack >> $ps
