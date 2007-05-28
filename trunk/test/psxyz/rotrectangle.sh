#!/bin/sh
#	$Id: rotrectangle.sh,v 1.2 2007-05-28 19:40:30 pwessel Exp $
#
# Test that psxyz properly plots rotatable rectangles -Sj and -SJ

echo -n "GMT: Test psxyz and the rotated rectangle option:		"

# Bottom case tests -SJ with azimuths and dimensions in km
cat << EOF > $$.rects.d
-65 15 0 90 500 200
-70 20 0 0 500 200
-80 25 0 70 500 300
EOF
psxyz -JZ2 -E135/30 -R270/20/305/25/0/3r -JOc280/25.5/22/69/5i -P -B10g5WSne -SJ $$.rects.d -Gred -W0.25p,green -K > rect.ps
psxyz -JZ -E135/30 -R -J -O -K -Sc0.05i $$.rects.d -Gblack >> rect.ps
# Middle case tests -Sj with angles and dimensions in inches (hence trailing i)
cat << EOF > $$.rects.d
-75 15 0 0 1 0.5
-70 20 0 30 1 0.5
-80 25 0 90 0.5 0.2
EOF
psxyz -JZ -E135/30 -R -J -O -K -B10g5WSne -Sji $$.rects.d -Gblue -W0.25p,green -Y3i >> rect.ps
psxyz -JZ -E135/30 -R -J -O -K -Sc0.05i $$.rects.d -Gblack >> rect.ps
# Top case is just Cartesian case where we pass angle and dimensions in -R units
cat << EOF > $$.rects.d
0 0 0 30 5 2
10 10 0 70 7 1
20 0 0 90 6 3
EOF
psxyz -JZ -E135/30 -R-10/25/-5/15/0/4 -Jx0.15i -O -K -B10g5WSne -SJ $$.rects.d -Gbrown -W0.25p,green -Y3i >> rect.ps
psxyz -JZ -E135/30 -R -J -O -Sc0.05i $$.rects.d -Gblack >> rect.ps
compare -density 100 -metric PSNR rect_orig.ps rect.ps rect_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAILED]"
else
        echo "[OK"]
        rm -f fail rect_diff.png log
fi
rm -f $$.*
