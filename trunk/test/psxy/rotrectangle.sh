#!/bin/bash
#	$Id$
#
# Test that psxy properly plots rotatable rectangles -Sj and -SJ

header "Test psxy and the rotated rectangle option"

# Bottom case tests -SJ with azimuths and dimensions in km
cat << EOF > tt.rects.d
-65 15 90 500 200
-70 20 0 500 200
-80 25 70 500 300
EOF
psxy -R270/20/305/25r -JOc280/25.5/22/69/4i -P -B10g5WSne -SJ tt.rects.d -Gred -W0.25p,green -K > $ps
psxy -R -J -O -K -Sc0.05i tt.rects.d -G0 >> $ps
# Middle case tests -Sj with angles and dimensions in inches (hence trailing i)
cat << EOF > tt.rects.d
-75 15 0 1 0.5
-70 20 30 1 0.5
-80 25 90 0.5 0.2
EOF
psxy -R -J -O -K -B10g5WSne -Sji tt.rects.d -Gblue -W0.25p,green -Y3i >> $ps
psxy -R -J -O -K -Sc0.05i tt.rects.d -G0 >> $ps
# Top case is just Cartesian case where we pass angle and dimensions in -R units
cat << EOF > tt.rects.d
0 0 30 5 2
10 10 70 7 1
20 0 90 6 3
EOF
psxy -R-10/25/-5/15 -Jx0.15i -O -K -B10g5WSne -SJ tt.rects.d -Gbrown -W0.25p,green -Y3i >> $ps
psxy -R -J -O -Sc0.05i tt.rects.d -G0 >> $ps

pscmp
