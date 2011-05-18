#! /bin/bash
#	$Id: cm4.sh,v 1.1 2011-05-18 23:36:15 jluis Exp $
#

. ../functions.sh
header "Test mgd77magref by comparing to one day data at a magnetic observatory"

ps=CM4.ps

gmtset FORMAT_DATE_MAP "o dd" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p
dia=clf20010501d.min

# Get Station location
lon=`tail -n +6 $dia | head -1 | awk '{print $3}'`
lat=`tail -n +5 $dia | head -1 | awk '{print $3}'`
alt=`tail -n +7 $dia | head -1 | awk '{print $2/1000}'`
data=`tail -n +27 $dia | head -1 | awk '{print $1}'`

echo $lon $lat $alt $data
IGRF=`echo $lon $lat $alt $data | mgd77magref -Fxyz/0`

tail -n +27 $dia | awk '{print $1"T"$2, sqrt($4*$4+$5*$5+$6*$6)}' > zz1.dat
awk -v x=$lon -v y=$lat -v z=$alt '{print x, y, z, $1}' zz1.dat | mgd77magref -Frt/923456 | awk '{print $4, $5}' > zz2.dat 
# Compute data & model min/max
m1=(`minmax zz1.dat -C -f0T`)
m2=(`minmax zz2.dat -C -f0T`)

# Compute limits such that both Y axes have the same scale
max_Y=`echo ${m1[2]} ${m1[3]} ${m2[2]} ${m2[3]} | awk '{if (($2-$1) > ($4-$3)) print($2-$1); else print($4-$3)}'`
y1_max=`echo ${m1[2]} $max_Y | awk '{print $1 + $2}'`
y2_max=`echo ${m2[2]} $max_Y | awk '{print $1 + $2}'`
psxy zz1.dat -R${m1[0]}/${m1[1]}/${m1[2]}/$y1_max -JX16c/6c -Bpa6Hf1h/10WSn -W1 -Y5.0c -P -K > $ps
psxy zz2.dat -R${m2[0]}/${m2[1]}/${m2[2]}/$y2_max -J -Bpa6Hf1h/10E -W1,red --BASEMAP_FRAME_RGB=+255/0/0 -O -K >> $ps

gmtmath zz1.dat zz2.dat SUB -F1 = dif_T.dat
std=`gmtmath dif_T.dat STD -S = `
mean=`gmtmath dif_T.dat MEAN -S = `

# Write Date, MEAN & STD
t=(`echo ${m2[2]} | awk '{print $1 + 4, $1 + 7, $1+12}'`)
echo ${m1[0]} ${t[1]} 11 0 17 LB Mean = $mean | pstext -R -J -N -X0.5 -O -K >> $ps
echo ${m1[0]} ${t[0]} 11 0 17 LB STD = $std | pstext -R -J -N -O -K >> $ps
echo ${m1[0]} ${t[2]} 12 0 17 LB $data  | pstext -R -J -N -O -K >> $ps
station=`tail -n +4 $dia | head -1 | awk '{print $3}'`
echo ${m1[0]} ${t[0]} 14 0 17 CT Station -- $station | pstext -R -J -N -Xa7.5c -Ya4.7c -O -K >> $ps

# Compute and write the IGRF for this day
IGRF=`echo $lon $lat $alt $data | mgd77magref -Ft/0`
echo ${m1[0]} ${t[0]} 10 0 17 CT IGRF = $IGRF | pstext -R -J -N -Xa7.5c -Ya3.0c -O -K >> $ps

# Plot histogram of differences with mean removed
gmtmath dif_T.dat $mean SUB = | pshistogram -F -W2 -G0 -JX4c/3c -BWN -Xa11.5c -O -K >> $ps

rm -f zz1.dat zz2.dat

#pscmp
