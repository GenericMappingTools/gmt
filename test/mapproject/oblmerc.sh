#!/bin/bash
#	$Id$
#
# Tests mapproject for oblique Mercator -R-20/40/-15/65r -Joa-30/60/-75/1:30000000 

gmtset MAP_ANNOT_OBLIQUE 0 FORMAT_GEO_MAP dddF
ps=oblmerc.ps
lon=-30
lat=60
az=-75
plon=132.807876266
plat=28.8790940174
scale=1:30000000
LL_lon=-56
LL_lat=51
UR_lon=20
UR_lat=50
# Find projected coordinates of LL and UR points in desired projection
echo $LL_lon $LL_lat | mapproject -C -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az}/$scale >tmp
LL_x=`cut -f1 tmp`
LL_y=`cut -f2 tmp`
echo $UR_lon $UR_lat | mapproject -C -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az}/$scale >tmp
UR_x=`cut -f1 tmp`
UR_y=`cut -f2 tmp`
# Create rectangle in these projected units
xstart=`gmtmath -Q $LL_x $UR_x MIN =`
xstop=`gmtmath -Q $LL_x $UR_x MAX =`
ystart=`gmtmath -Q $LL_y $UR_y MIN =`
ystop=`gmtmath -Q $LL_y $UR_y MAX =`
gmtmath -T$xstart/$xstop/101+ $LL_y = > box.xy
gmtmath -o1,0 -T$ystart/$ystop/101+ $UR_x = >> box.xy
gmtmath -I -T$xstart/$xstop/101+ $UR_y = >> box.xy
gmtmath -I -o1,0 -T$ystart/$ystop/101+ $LL_x = >> box.xy
# Get back degrees
mapproject -C -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az}/$scale -I box.xy > box.d
# Use -Joa to set origin and azimuth.
psbasemap -Rg -JG0/80/6i -Bafg30 -P -K -Xc -Y4.5i > $ps
psbasemap -R -J -Bafg30+o${plon}/${plat} -O -K --MAP_GRID_PEN_PRIMARY=0.25p,. >> $ps
psxy -R -J box.d -Glightgreen -W0.5p -O -K >> $ps
echo $lon $lat 0 200 200 | psxy -R -J -O -K -SE -Gblack >> $ps
project -C$lon/$lat -G25 -A-75 -L-20000/20000 -Q | psxy -R -J -O -K -W0.25p,blue >> $ps
project -C$lon/$lat -G25 -A15 -L-20000/20000 -Q | psxy -R -J -O -K -W0.25p,blue >> $ps
echo $lon $lat -75 1000 | psxy -R -J -O -K -S=0.1i+e -Gred -W0.5p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $lon $lat -165 1000 | psxy -R -J -O -K -S=0.1i+e -Gred -W0.5p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $plon $plat 0 200 200 | psxy -R -J -O -K -SE -Gred >> $ps
pstext -R -J -O -K -F+f12p+j -Dj0.1i << EOF >> $ps
-42 63 CM x
-29 53 CM y
129 26 CM P
$LL_lon $LL_lat RB LL
$UR_lon $UR_lat TL UR
EOF
echo $LL_lon $LL_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
echo $UR_lon $UR_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
#
psbasemap  -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az}/$scale -O -K -Bafg+glightgreen -Y-3.5i >> $ps
project -C$lon/$lat -G25 -A-75 -L-20000/20000 -Q | psxy -R -J -O -K -W0.25p,blue >> $ps
project -C$lon/$lat -G25 -A15 -L-2000/2000 -Q | psxy -R -J -O -K -W0.25p,blue >> $ps
echo $lon $lat 0 100 100 | psxy -R -J -O -K -SE -Gblack >> $ps
echo $lon $lat -75 1000 | psxy -R -J -O -K -S=0.15i+e -Gred -W1p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $lon $lat -165 1000 | psxy -R -J -O -K -S=0.15i+e -Gred -W1p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $LL_lon $LL_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
echo $UR_lon $UR_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
pstext -R -J -O -K -F+f12p+j -Dj0.1i -N << EOF >> $ps
-42 62 CM x
-31 52 CM y
$LL_lon $LL_lat RB LL
$UR_lon $UR_lat TL UR
EOF

psxy -R -J -O -T >> $ps

