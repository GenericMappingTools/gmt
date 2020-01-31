#!/usr/bin/env bash
# Testing a S polar cap using -JG in a variety of views
ps=gview_caps.ps
gmt grdmath -R0.5/6.5/0.5/9.5 -I1.5 XCOL 36 MUL 72 SUB = lat.nc
gmt grdmath -R0.5/6.5/0.5/9.5 -I1.5 YROW 50 MUL 5 ADD  = lon.nc
gmt grd2xyz lat.nc > lat.txt
gmt grd2xyz lon.nc > lon.txt
gmt convert -A lon.txt lat.txt -o0:2,5 > tmp.txt
gmt pstext -R0/8.5/0/11 -Jx1i -P -Xa0 -Ya0 -K -F+f10+j -N << EOF > $ps
1.25    0.45	CT	72S
2.75	0.45	CT	36S
4.25	0.45	CT	0
5.75	0.45	CT	36N
7.25	0.45	CT	72S
0.45	10.25	RM	5E
0.45	8.75	RM	55E
0.45	7.25	RM	105E
0.45	5.75	RM	155E
0.45	4.25	RM	155W
0.45	2.75	RM	105W
0.45	1.25	RM	55W
EOF
while read x y lon lat; do
	gmt pscoast -Rd -JG${lon}/${lat}/1.45i -Glightgray -Xa${x}i -Ya${y}i -O -K -Dc >> $ps
	gmt psxy pol_S.txt -Rd -J -Gred -Xa${x}i -Ya${y}i -O -K >> $ps
	gmt psxy pol_N.txt -Rd -J -Gblue -Bg30 -Xa${x}i -Ya${y}i -O -K >> $ps
done < tmp.txt
gmt psxy -R -J -O -T >> $ps
