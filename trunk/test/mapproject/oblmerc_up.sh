#!/bin/bash
#	$Id$
#
# Tests mapproject for oblique Mercator -R-20/40/-15/65r -Joa-30/60/105/1:30000000 

gmtset MAP_ANNOT_OBLIQUE 0 FORMAT_GEO_MAP dddF
ps=oblmerc_up.ps
lon=-30
lat=60
az_x=105
az_y=`gmtmath -Q $az_x 90 SUB =`
plon=`gmtvector -A$lon/$lat -Tp$az_x} | cut -f1`
plat=`gmtvector -A$lon/$lat -Tp$az_x} | cut -f2`
scale=1:30000000
scale_km=1:30000
LL_lon=-56
LL_lat=51
UR_lon=20
UR_lat=50
# Find projected coordinates of LL and UR points in desired projection
echo $LL_lon $LL_lat | mapproject -C -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az_x}/$scale -Fk > tmp
LL_x=`cut -f1 tmp`
LL_y=`cut -f2 tmp`
echo $UR_lon $UR_lat | mapproject -C -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az_x}/$scale -Fk > tmp
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
mapproject -C -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az_x}/$scale -I -Fk box.xy > box.d
# Use -Joa to set origin and azimuth.
psxy -Rg -JG0/80/6i -Bafg30 box.d -Glightgreen -W0.5p -P -K -Xc -Y4.25i > $ps
psbasemap -R -J -Bafg30+o${plon}/${plat} -O -K --MAP_GRID_PEN_PRIMARY=0.25p,. >> $ps
echo $lon $lat 0 200 200 | psxy -R -J -O -K -SE -Gblack >> $ps
project -C$lon/$lat -G25 -A$az_x -L-20000/20000 -Q | psxy -R -J -O -K -W0.5p,blue >> $ps
project -C$lon/$lat -G25 -A$az_y -L-20000/20000 -Q | psxy -R -J -O -K -W0.5p,blue >> $ps
echo $lon $lat $az_x 800 | psxy -R -J -O -K -S=0.1i+e -Gred -W0.5p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $lon $lat $az_y 800 | psxy -R -J -O -K -S=0.1i+e -Gred -W0.5p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $plon $plat 0 200 200 | psxy -R -J -O -K -SE -Gred >> $ps
pstext -R -J -O -K -F+f12p+j+a-50 -Dj0.1i << EOF >> $ps
-19 55 CM x
-31 66 CM y
129 26 CM P
$LL_lon $LL_lat RB LL
$UR_lon $UR_lat TL UR
EOF
echo $LL_lon $LL_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
echo $UR_lon $UR_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
#
psbasemap  -R$LL_lon/$LL_lat/$UR_lon/${UR_lat}r -Joa${lon}/${lat}/${az_x}/$scale -O -K -BafgWS -B+glightgreen -Y-3.5i >> $ps
project -C$lon/$lat -G25 -A$az_x -L-20000/20000 -Q | psxy -R -J -O -K -W0.5p,blue >> $ps
project -C$lon/$lat -G25 -A$az_y -L-2000/2000 -Q | psxy -R -J -O -K -W0.5p,blue >> $ps
echo $lon $lat 0 100 100 | psxy -R -J -O -K -SE -Gblack >> $ps
echo $lon $lat $az_x 800 | psxy -R -J -O -K -S=0.15i+e -Gred -W1p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $lon $lat $az_y 800 | psxy -R -J -O -K -S=0.15i+e -Gred -W1p,red --MAP_VECTOR_SHAPE=0.5 >> $ps
echo $LL_lon $LL_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
echo $UR_lon $UR_lat 0 150 150 | psxy -R -J -O -K -SE -Gblue >> $ps
pstext -R -J -O -K -F+f12p+j -Dj0.1i -N << EOF >> $ps
-19 57 CM x
-29 66 CM y
$LL_lon $LL_lat RB LL
$UR_lon $UR_lat TL UR
EOF
psbasemap -R$LL_x/$LL_y/$UR_x/${UR_y}r -Jx$scale_km -O -K -BafgNE --MAP_GRID_PEN_PRIMARY=0.25p,. >> $ps
echo "0 9.5 az = $az_x" | pstext -R0/8/0/10 -Jx1i -O -K -F+f24p+jTL >> $ps
psxy -R -J -O -T >> $ps

