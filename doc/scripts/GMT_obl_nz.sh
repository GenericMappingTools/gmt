#!/bin/bash
# Oblique Mercator map for NZ using complementary poles
ps=GMT_obl_nz.ps
lon=173:17:02E
lat=41:16:15S
az=35
w=2000
h=1000
plon=180
plat=40S
# Centered
w2=`gmt math -Q $w 2 DIV =`
h2=`gmt math -Q $h 2 DIV =`
R=-Rk-${w2}/$w2/-${h2}/$h2
gmt pscoast $R -JOA$lon/$lat/$az/3i -Ba5f5g5 -Gred -Dh -P -K -TdjBR+w0.5i+l+o0.2i/-0.05i --FONT_TITLE=9p --MAP_ANNOT_OBLIQUE=34 --FORMAT_GEO_MAP=dddF > $ps
echo $plon $plat | gmt psxy -R -J -O -K -Sc0.2c -Gblue -W0.25p >> $ps
gmt psxy -R0/3/0/1.5 -Jx1i -O -K -W0.25p,- << EOF >> $ps
>
0	0.75
3	0.75
>
1.5	0
1.5	1.5
EOF
gmt psxy -R -J -O -K -Sv0.2i+e+h0.5 -Gblack -W2p -N << EOF >> $ps
1.5	0.75	0	1.5i
1.5	0.75	90	0.75i
EOF
echo 1.5 0.75 | gmt psxy -R -J -O -K -Sc0.1c -Wfaint -Gwhite >> $ps
gmt pstext -R -J -O -K -F+f12p,Times-Italic+j -Dj0.1i -Gwhite << EOF >> $ps
3	0.75	TR	x
1.5	1.5	TR	y
EOF
echo $plon $plat | gmt mapproject -R -JoA$lon/$lat/$az/1:1 -Fk -C
az=215
gmt pscoast $R -JOA$lon/$lat/$az/3i -Ba5f5g5 -Gred -Dh -O -K -X3.4i -TdjTL+w0.5i+l+o0.2i/-0.05i --FONT_TITLE=9p --MAP_ANNOT_OBLIQUE=34 --FORMAT_GEO_MAP=dddF >> $ps
echo $plon $plat | gmt psxy -R -J -O -K -Sc0.2c -Gblue -W0.25p >> $ps
gmt psxy -R0/3/0/1.5 -Jx1i -O -K -W0.25p,- << EOF >> $ps
>
0	0.75
3	0.75
>
1.5	0
1.5	1.5
EOF
gmt psxy -R -J -O -K -Sv0.2i+e+h0.5 -Gblack -W2p -N << EOF >> $ps
1.5	0.75	0	1.5i
1.5	0.75	90	0.75i
EOF
echo 1.5 0.75 | gmt psxy -R -J -O -K -Sc0.1c -Wfaint -Gwhite >> $ps
gmt pstext -R -J -O -F+f12p,Times-Italic+j -Dj0.1i -Gwhite << EOF >> $ps
3	0.75	TR	x
1.5	1.5	TR	y
EOF
echo $plon $plat | gmt mapproject -R -JoA$lon/$lat/$az/1:1 -Fk -C
