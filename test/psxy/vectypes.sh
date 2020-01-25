#!/usr/bin/env bash
# Vector components (dx, dy)
# The given vectors (0,10) and (10,0) has length 10.  So a length of 10 is mapped to a length on the plot
# psxy -SV..+z0.1: Scale 0.1 should yield a 1 plot unit long vector, we set unit to inch for 1i.
# Vector aximuth and length (az, length)
# The other vectors (0,100) and (90,100) has length 100.  So a length of 100 is mapped to a length on the plot
# psxy -i..3+s0.01. Scale 0.01 should yield a 1 plot unit long vector, we set unit to inch for 1i.
# These are equivalent and same length at both latitudes, showing that either vector specification works.
# We also ocompare with geovector, plotting vectors of length 1 degree = 111.13 km which are longer on the
# map at higher latitudes, as expected.
ps=vectypes.ps
# Scale ector components via +z
echo -2	0	0	10 > t.txt
echo -2	0	10	0 >> t.txt
gmt psxy t.txt -JM6i -R-4/4/-2/2 -SV0.3i+e+z0.1 -W2p,red -Gred -P -Bafg1 -BWSne -K -Xc --PROJ_LENGTH_UNIT=inch > $ps
echo -2	60	0	10 > t.txt
echo -2	60	10	0 >> t.txt
gmt psxy t.txt -J -R-4/4/58/62 -SV0.3i+e+z0.1 -W2p,red -Gred -Bafg1 -BWsne -O -K -Y3.3i --PROJ_LENGTH_UNIT=inch >> $ps
# Scale vector lengths via -i..+s
echo 2	0	0	100 > t.txt
echo 2	0	90	100 >> t.txt
gmt psxy t.txt -JM6i -R-4/4/-2/2 -SV0.3i+e -i0:2,3+s0.01 -W2p,blue -Gblue -O -K -Y-3.3i --PROJ_LENGTH_UNIT=inch >> $ps
echo 2	60	0	100 > t.txt
echo 2	60	90	100 >> t.txt
gmt psxy t.txt -J -R-4/4/58/62 -SV0.3i+e -i0:2,3+s0.01 -W2p,blue -Gblue -O -K -Y3.3i --PROJ_LENGTH_UNIT=inch >> $ps
# Try geovector on these 111.13 km vectors pointing east and north
echo 0	0	0	111.13 > t.txt
echo 0	0	90	111.13 >> t.txt
gmt psxy t.txt -JM6i -R-4/4/-2/2 -S=0.3i+e -W2p,green -Ggreen -O -K -Y-3.3i --PROJ_LENGTH_UNIT=inch >> $ps
echo 0	60	0	111.13 > t.txt
echo 0	60	90	111.13 >> t.txt
gmt psxy t.txt -J -R-4/4/58/62 -S=0.3i+e -W2p,green -Ggreen -O -K -Y3.3i --PROJ_LENGTH_UNIT=inch >> $ps
gmt psxy -R -J -O -T >> $ps
