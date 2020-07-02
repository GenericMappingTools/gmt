#!/usr/bin/env bash
# Oblique Mercator map for NZ using complementary poles
gmt begin GMT_obl_nz
lon=173:17:02E
lat=41:16:15S
az=35
w=2000
h=1000
plon=180
plat=40S
# Centered
w2=$(gmt math -Q $w 2 DIV =)
h2=$(gmt math -Q $h 2 DIV =)
R=-R-${w2}/$w2/-${h2}/${h2}+uk
gmt coast $R -JOA$lon/$lat/$az/3i -Ba5f5g5 -Gred -Dh -TdjBR+w0.5i+l+o0.2i/-0.05i --FONT_TITLE=9p --MAP_ANNOT_OBLIQUE=separate,lon_horizontal,lat_parallel --FORMAT_GEO_MAP=dddF
echo $plon $plat | gmt plot -Sc0.2c -Gblue -W0.25p
gmt plot -R0/3/0/1.5 -Jx1i -W0.25p,- << EOF
>
0	0.75
3	0.75
>
1.5	0
1.5	1.5
EOF
gmt plot -Sv0.2i+e+h0.5 -Gblack -W2p -N << EOF
1.5	0.75	0	1.5i
1.5	0.75	90	0.75i
EOF
echo 1.5 0.75 | gmt plot -Sc0.1c -Wfaint -Gwhite
gmt text -F+f12p,Times-Italic+j -Dj0.1i -Gwhite << EOF
3	0.75	TR	x
1.5	1.5	TR	y
EOF
#echo $plon $plat | gmt mapproject -JoA$lon/$lat/$az/1:1 -Fk -C
az=215
gmt coast $R -JOA$lon/$lat/$az/3i -Ba5f5g5 -Gred -Dh -X3.4i -TdjTL+w0.5i+l+o0.2i/-0.05i --FONT_TITLE=9p --MAP_ANNOT_OBLIQUE=separate,lon_horizontal,lat_parallel --FORMAT_GEO_MAP=dddF
echo $plon $plat | gmt plot -Sc0.2c -Gblue -W0.25p
gmt plot -R0/3/0/1.5 -Jx1i -W0.25p,- << EOF
>
0	0.75
3	0.75
>
1.5	0
1.5	1.5
EOF
gmt plot -Sv0.2i+e+h0.5 -Gblack -W2p -N << EOF
1.5	0.75	0	1.5i
1.5	0.75	90	0.75i
EOF
echo 1.5 0.75 | gmt plot -Sc0.1c -Wfaint -Gwhite
gmt text -F+f12p,Times-Italic+j -Dj0.1i -Gwhite << EOF
3	0.75	TR	x
1.5	1.5	TR	y
EOF
#echo $plon $plat | gmt mapproject -JoA$lon/$lat/$az/1:1 -Fk -C
gmt end show
