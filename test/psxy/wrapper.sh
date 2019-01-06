#!/usr/bin/env bash
# Examples of how large polygons (> 180 degree range) clipped by near-global regions
# sometimes fails the inside/outside area tests.
# There is not yet a orig to compare but it fails.
ps=wrapper.ps
# bad1 fails as the filling is the opposite of expected.
# This polygon has 210 degrees longitudinal extent
# When not clipped (top region plot) it works
cat << EOF > bad1.txt
90	0
200	0
240	0
285	0
300	0
300	-40
285	-40
240	-40
200	-40
90	-40
90	0
EOF
# bad2 is clipped, filled, and outlined correctly.  It
# has a shorter extent than bad1 (40 degrees).
cat << EOF > bad2.txt
90	20
130	20
130	50
90	50
90	20
EOF
# A region that fails
gmt psxy -R-110/120/-60/60 -JM6i -Glightred -P -Baf -K bad1.txt -A -Xc > $ps
gmt psxy -R -J -Glightblue -O -K bad2.txt -A >> $ps
gmt psxy -R -J -O -K -W1p bad1.txt bad2.txt -A >> $ps
# A region that works
gmt psxy -R80/310/-60/60 -J -Glightred -O -Baf -K bad1.txt -A -Y5i >> $ps
gmt psxy -R -J -Glightblue -O -K bad2.txt -A >> $ps
gmt psxy -R -J -O -W1p bad1.txt bad2.txt -A >> $ps
