#!/bin/bash
#	$Id$
#
# Making sure both forms of "near a line" works:
# Default (old) behavior is to think of a line as
# a continuous trace of points; hence data points
# within the given distance from the end points are
# considered inside even if they project to the
# extension of the line segment.
# Optional (new) behaviour (-Lp) will only consider
# points near the line if they project inside the
# line's endpoints

. ./functions.sh
header "Test gmtselect's new -L[p] option on given data"

# Some test data
grdmath -R0/5/0/5 -I0.1 0 = tt.nc
cat << EOF > tt.d
> line 1
1 1
2 2.2
3 2.8
3.7 4
EOF
grd2xyz tt.nc > tt.xyz
# Do test both with Cartesian and spherical data
# CARTESIAN DATA: distance D = 1 unit
D=1
# Old behavior
psxy -R0/5/0/5 -JX3.25i -P -B1g1WSne -K -Sc0.02 -Gred tt.xyz -X0.75i -Y1i > $ps
gmtselect tt.xyz -L${D}/tt.d | psxy -R -J -O -K -Sc0.02 -Ggreen >> $ps
psxy -R -J -O -K tt.d -W1p >> $ps
# New behavior
psxy -R -J -O -B1g1WSne -K -Sc0.02 -Gred tt.xyz -X3.75i >> $ps
gmtselect tt.xyz -Lp${D}/tt.d | psxy -R -J -O -K -Sc0.02 -Ggreen >> $ps
psxy -R -J -O -K tt.d -W1p >> $ps
# SPHERICAL DATA (-fg): distance D = 1 degree
D=1d
# Old behavior
psxy -R -JM3.25i -O -B1g1WSne -K -Sc0.02 -Gred tt.xyz -X-3.75i -Y4i >> $ps
gmtselect tt.xyz -L${D}/tt.d -fg | psxy -R -J -O -K -Sc0.02 -Ggreen >> $ps
psxy -R -J -O -K tt.d -W1p >> $ps
# New behavior
psxy -R -J -O -B1g1WSne -K -Sc0.02 -Gred tt.xyz -X3.75i >> $ps
gmtselect tt.xyz -Lp${D}/tt.d -fg | psxy -R -J -O -K -Sc0.02 -Ggreen >> $ps
psxy -R -J -O tt.d -W1p >> $ps

pscmp
