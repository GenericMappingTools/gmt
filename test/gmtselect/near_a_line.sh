#!/usr/bin/env bash
#
# Making sure both forms of "near a line" works:
# Default (old) behavior is to think of a line as
# a continuous trace of points; hence data points
# within the given distance from the end points are
# considered inside even if they gmt project to the
# extension of the line segment.
# Optional (new) behavior (-Lp) will only consider
# points near the line if they gmt project inside the
# line's endpoints

ps=near_a_line.ps

# Some test data
gmt grdmath -R0/5/0/5 -I0.1 0 = tt.nc
cat << EOF > tt.d
> line 1
1 1
2 2.2
3 2.8
3.7 4
EOF
gmt grd2xyz tt.nc > tt.xyz
# Do test both with Cartesian and spherical data
# CARTESIAN DATA: distance D = 1 unit
D=1
# Old behavior
gmt psxy -R0/5/0/5 -JX3.25i -P -K -Sc0.02 -Gred tt.xyz -X0.75i -Y1i > $ps
gmt select tt.xyz -Ltt.d+d${D} | gmt psxy -R -J -O -K -B1g1 -BWSne -Sc0.02 -Ggreen >> $ps
gmt psxy -R -J -O -K tt.d -W1p >> $ps
# New behavior
gmt psxy -R -J -O -K -Sc0.02 -Gred tt.xyz -X3.75i >> $ps
gmt select tt.xyz -Ltt.d+d${D}+p | gmt psxy -R -J -O -K -B1g1 -BWSne -Sc0.02 -Ggreen >> $ps
gmt psxy -R -J -O -K tt.d -W1p >> $ps
# SPHERICAL DATA (-fg): distance D = 1 degree
D=1d
# Old behavior
gmt psxy -R -JM3.25i -O -K -Sc0.02 -Gred tt.xyz -X-3.75i -Y4i >> $ps
gmt select tt.xyz -Ltt.d+d${D} -fg | gmt psxy -R -J -O -K -B1g1 -BWSne -Sc0.02 -Ggreen >> $ps
gmt psxy -R -J -O -K tt.d -W1p >> $ps
# New behavior
gmt psxy -R -J -O -K -Sc0.02 -Gred tt.xyz -X3.75i >> $ps
gmt select tt.xyz -Ltt.d+d${D}+p -fg | gmt psxy -R -J -O -K -B1g1 -BWSne -Sc0.02 -Ggreen >> $ps
gmt psxy -R -J -O tt.d -W1p >> $ps
