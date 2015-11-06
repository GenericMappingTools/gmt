#!/bin/bash
# Test psmask and oriented clip paths polygons
ps=masking.ps

function arrow() {
	# Given file, pull out first 2 records and draw an arrow from first to second point per segment
	gmt convert $1 -Dtmp.%d
	for file in tmp.*; do
		first=`sed -n 2p $file`
		second=`sed -n 3p $file`
		echo $first $second | awk '{print $1, $2, $3, $4}' | gmt psxy -R -J -O -K -Sv0.15i+e+s -Gblack
	done
	rm -f tmp.*
}
# Some x,y data points, z is arbitrary and not used here */
cat << EOF > data.txt
2.00	5.99	3.03
2.06	6.98	3.12
1.95	7.86	2.01
2.87	3.92	2.89
3.02	4.96	3.13
2.83	6.14	2.98
3.23	6.87	2.93
3.05	7.81	2.96
3.88	3.03	2.97
4.23	3.94	2.93
4.00	5.04	2.99
3.96	5.86	3.02
4.14	6.97	3.04
3.97	8.03	3.02
5.07	2.97	3.03
5.03	4.00	2.96
4.91	6.88	2.94
4.98	7.90	3.14
5.99	1.96	3.06
5.95	3.09	2.98
5.94	4.00	3.08
6.03	6.99	3.10
5.92	7.60	3.17
6.94	3.00	3.08
7.04	3.95	2.87
7.01	5.01	2.97
7.14	6.13	3.14
7.03	7.23	3.05
6.95	8.05	3.03
8.14	2.99	2.98
7.89	3.91	3.15
8.01	4.95	3.03
8.07	6.09	3.05
7.95	7.06	2.97
EOF
Ri=-R0/10/0/10
Ro=-R-1/11/-1/11
gmtset MAP_VECTOR_SHAPE 0.5
gmt psxy $Ro -JX3i -P -Bafg1 -BWSne data.txt -Ss0.1i -Gblue -W0.25p -K -Y0.5i > $ps
gmt psmask $Ri -J -I1 data.txt -Dclip.txt -L+psmask.nc
gmt grd2xyz -s psmask.nc | psxy $Ro -J -O -K -Sc0.03i -Gred >> $ps
gmt psxy $Ro -J -O -K clip.txt -W1p,red >> $ps
echo 0 0 Unoriented | gmt pstext -R -J -O -K -F+f12p+jLB -Gwhite -W0.5p >> $ps
arrow clip.txt >> $ps
gmt psxy $Ro -J -O -Bafg1 -BwSnE data.txt -Ss0.1i -Gblue -W0.25p -K -X3.25i >> $ps
gmt psmask $Ri -J -I1 data.txt -Dclipf.txt -Fr -L+psmask.nc
gmt grd2xyz -s psmask.nc | psxy $Ro -J -O -K -Sc0.03i -Gred >> $ps
gmt psxy $Ro -J -O -K clipf.txt -W1p,red >> $ps
echo 0 0 Oriented -Fr | gmt pstext -R -J -O -K -F+f12p+jLB -Gwhite -W0.5p >> $ps
arrow clipf.txt >> $ps
gmt psxy $Ro -J -O -Bafg1 -BWSne data.txt -Ss0.1i -Gblue -W0.25p -K -X-3.25i -Y3.4i >> $ps
gmt psmask $Ri -J -I1 data.txt -Dclipn.txt -N -L+psmask.nc
gmt grd2xyz -sa psmask.nc | psxy $Ro -J -O -K -Sc0.03i -Gred >> $ps
gmt psxy $Ro -J -O -K clipn.txt -W1p,red >> $ps
echo 0 0 Unoriented | gmt pstext -R -J -O -K -F+f12p+jLB -Gwhite -W0.5p >> $ps
arrow clipn.txt >> $ps
gmt psxy $Ro -J -O -Bafg1 -BwSnE data.txt -Ss0.1i -Gblue -W0.25p -K -X3.25i >> $ps
gmt psmask $Ri -J -I1 data.txt -Dclipnf.txt -Fl -N -L+psmask.nc
gmt grd2xyz -sa psmask.nc | psxy $Ro -J -O -K -Sc0.03i -Gred >> $ps
gmt psxy $Ro -J -O -K clipnf.txt -W1p,red >> $ps
echo 0 0 Oriented -Fl | gmt pstext -R -J -O -K -F+f12p+jLB -Gwhite -W0.5p >> $ps
arrow clipnf.txt >> $ps
gmt psxy $Ro -J -O -Bafg1 -BWSNe data.txt -Ss0.1i -Gblue -W0.25p -K -X-3.25i -Y3.4i >> $ps
gmt psmask $Ri -J -I1 data.txt -Dclipns.txt -S1 -N -L+psmask.nc
gmt grd2xyz -s psmask.nc | psxy $Ro -J -O -K -Sc0.03i -Gred >> $ps
gmt psxy $Ro -J -O -K clipns.txt -W1p,red >> $ps
echo 0 0 Unoriented -S1 | gmt pstext -R -J -O -K -F+f12p+jLB -Gwhite -W0.5p >> $ps
arrow clipns.txt >> $ps
gmt psxy $Ro -J -O -Bafg1 -BwSNE data.txt -Ss0.1i -Gblue -W0.25p -K -X3.25i >> $ps
gmt psmask $Ri -J -I1 data.txt -Dclipfs.txt -Fl -L+psmask.nc
gmt grd2xyz -s psmask.nc | psxy $Ro -J -O -K -Sc0.03i -Gred >> $ps
gmt psxy $Ro -J -O -K clipfs.txt -W1p,red >> $ps
echo 0 0 Oriented -Fl | gmt pstext -R -J -O -K -F+f12p+jLB -Gwhite -W0.5p >> $ps
arrow clipfs.txt >> $ps
gmt psxy -R -J -O -T >> $ps
