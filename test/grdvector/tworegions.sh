#!/usr/bin/env bash
# The given vector (0,10) has length 10.  So a length of 10 is mapped to a length on the plot
# psvelo: Scale 0.1i should yield a 1i long vector
# grdvector: Scale 0.1i should yield a 1i long vector.  However, until we fix things, scales
# for geographic vectors are only given as data units / km, and then the length in km is mapped
# to plot lengths which will vary with projection and location.  Hence the blue vectors plotted
# by grdvector have different plot lengths despite both having the same user length of 10.
# Will update this test to supply an original once we fix it.
# 22-JUL-2018: Fixed and uploaded an original.
ps=tworegions.ps
echo 0	0	0	10	1	1  0 > t.txt
gmt xyz2grd -R-1/1/-1/1 -I30m -Gu.grd t.txt
gmt xyz2grd -R -I30m -Gv.grd t.txt -i0,1,3
gmt psvelo t.txt -JM3i -R-2/2/-2/2 -Se0.1i/0.95/12 -A9p+e -W1.5p,red -P -Bafg1 -BWSne -K > $ps
gmt grdvector u.grd v.grd -R -J -O -K -Q0.1i+e -Gblue -W1p,blue -Si0.1i -Bafg1 -BwSne --MAP_VECTOR_SHAPE=0.2 -X3.25i >> $ps
gmt psvelo t.txt -J -R-1/1/-1/1 -Se0.1i/0.95/12 -A9p+e -W1.5p,red -O -Bafg1 -BWSne -K -X-3.25i -Y3.75i >> $ps
gmt grdvector u.grd v.grd -R -J -O -Q0.1i+e -Gblue -W1p,blue -Si0.1i -Bafg1 -BwSne --MAP_VECTOR_SHAPE=0.2 -X3.25i >> $ps
