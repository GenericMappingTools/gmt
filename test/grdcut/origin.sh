#!/bin/sh
#	$Id$
# Testing grdcut -S for 3 different points

ps=origin.ps

# Create global grid by evaluating distances to 0,0
grdmath -Rd -I1 0 0 SDIST = tmp.nc
grdcontour tmp.nc -JQ0/7i -P -K -A10 -C5 -Baf -Y6.5i -Xc > $ps
# Plot three origins and circles
psxy -R -J -O -K -SE << EOF >> $ps
> -W1p,red
30 30 0 3000 3000
> -W1p,green
-170 -60 0 3000 3000
> -W1p,blue
145 78 0 3000 3000
EOF
psxy -R -J -O -K -Sx0.1i << EOF >> $ps
> -W1p,red
30 30
> -W1p,green
-170 -60
> -W1p,blue
145 78
EOF
# 1st point
grdcut tmp.nc -S30/30/1500k -Gt.nc
grdcontour t.nc -JM3i -O -K -A10 -C5 -Baf -Y-4i -Gd2i >> $ps
echo 30 30 0 3000 3000 | psxy -Rt.nc -J -O -K -SE -W1p,red >> $ps
echo 30 30 | psxy -Rt.nc -J -O -K -Sx0.1i -W1p,red >> $ps
# 2nd point
grdcut tmp.nc -S-170/-60/1500k -Gt.nc
grdcontour t.nc -JM3i -O -K -A10 -C5 -Baf -X4i -Gd2i >> $ps
echo -170 -60 0 3000 3000 | psxy -Rt.nc -J -O -K -SE -W1p,green >> $ps
echo -170 -60 | psxy -Rt.nc -J -O -K -Sx0.1i -W1p,green >> $ps
# 3rd point
grdcut tmp.nc -S145/78/1500k -Gt.nc
grdcontour t.nc -JQ0/7i -O -K -A10 -C5 -Baf -Gd2i -X-4i -Y-1.5i >> $ps
echo 145 78 0 3000 3000 | psxy -Rt.nc -J -O -K -SE -W1p,blue >> $ps
echo 145 78 | psxy -Rt.nc -J -O -K -Sx0.1i -W1p,blue >> $ps

psxy -R -J -O -T >> $ps
