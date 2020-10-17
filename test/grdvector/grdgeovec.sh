#!/usr/bin/env bash
# Testing grdvectors geovector unit system with
# The given vector (0,111.13) has length 111.13 which is mapped to km on the map
# which are then projected to the corresponding lengths in plot units.  However,
# we can give other units, such as nautical or statute miles, and then these are
# converted to km, yielding longer vectors for the same user lengths.
ps=grdgeovec.ps
gmt set MAP_VECTOR_SHAPE 0.2 MAP_FRAME_TYPE plain
# First we say these are km
echo -1	0	0	111.13 > t.txt
echo 1	0	90	111.13 >> t.txt
gmt xyz2grd -R-1/1/-1/1 -I1 -fg -Ga.grd t.txt
gmt xyz2grd -R -I1 -Gr.grd -fg t.txt -i0,1,3
gmt grdvector r.grd a.grd -A -JM2i -R-2/3/-2/3 -P -K -Bafg1 -BWSne -Q0.2i+e -Gblue -W2p,blue -S1k > $ps
echo -1	60	0	111.13 > t.txt
echo 1	60	90	111.13 >> t.txt
gmt xyz2grd -R-1/1/59/61 -I1 -fg -Ga.grd t.txt
gmt xyz2grd -R -I1 -fg -Gr.grd t.txt -i0,1,3
gmt grdvector r.grd a.grd -A -R-2/3/58/63 -J -O -K -Q0.2i+e -Gblue -W2p,blue -S1k -Bafg1 -BWSne+t"km" -Y3.75i >> $ps
# Now we say these are miles statute
echo -1	0	0	111.13 > t.txt
echo 1	0	90	111.13 >> t.txt
gmt xyz2grd -R-1/1/-1/1 -I1 -fg -Ga.grd t.txt
gmt xyz2grd -R -I1 -Gr.grd -fg t.txt -i0,1,3
gmt grdvector r.grd a.grd -A -J -R-2/3/-2/3 -O -K -Bafg1 -BwSne -Q0.2i+e -Gblue -W2p,blue -S1M -X2.25i -Y-3.75i >> $ps
echo -1	60	0	111.13 > t.txt
echo 1	60	90	111.13 >> t.txt
gmt xyz2grd -R-1/1/59/61 -I1 -fg -Ga.grd t.txt
gmt xyz2grd -R -I1 -fg -Gr.grd t.txt -i0,1,3
gmt grdvector r.grd a.grd -A -R-2/3/58/63 -J -O -K -Q0.2i+e -Gblue -W2p,blue -S1M -Bafg1 -BwSne+t"M" -Y3.75i >> $ps
# Finally we say these are nautical milesx10 so we need a smaller scale of 0.1 unit per nm
echo -1	0	0	11.113 > t.txt
echo 1	0	90	11.113 >> t.txt
gmt xyz2grd -R-1/1/-1/1 -I1 -fg -Ga.grd t.txt
gmt xyz2grd -R -I1 -Gr.grd -fg t.txt -i0,1,3
gmt grdvector r.grd a.grd -A -J -R-2/3/-2/3 -O -K -Bafg1 -BwSne -Q0.2i+e -Gblue -W2p,blue -S0.1n -X2.25i -Y-3.75i >> $ps
echo -1	60	0	11.113 > t.txt
echo 1	60	90	11.113 >> t.txt
gmt xyz2grd -R-1/1/59/61 -I1 -fg -Ga.grd t.txt
gmt xyz2grd -R -I1 -fg -Gr.grd t.txt -i0,1,3
gmt grdvector r.grd a.grd -A -R-2/3/58/63 -J -O -Q0.2i+e -Gblue -W2p,blue -S0.1n -Bafg1 -BwSne+t"nm" -Y3.75i >> $ps
