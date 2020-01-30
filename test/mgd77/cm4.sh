#!/usr/bin/env bash
#
# Observatory magnetic data dl from ottawa.intermagnet.org/apps/dl_data_prel_e.php

ps=cm4.ps

gmt set FORMAT_DATE_MAP "o dd" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p
dia=$(gmt which -G @clf20010501d.min)

# Get Station location
lon=$(tail -n +6 $dia | head -1 | $AWK '{print $3}')
lat=$(tail -n +5 $dia | head -1 | $AWK '{print $3}')
alt=$(tail -n +7 $dia | head -1 | $AWK '{print $2/1000}')
data=$(tail -n +27 $dia | head -1 | $AWK '{print $1}')

IGRF=$(echo $lon $lat $alt $data | gmt mgd77magref -Fxyz/0)

tail -n +27 $dia | $AWK '{print $1"T"$2, sqrt($4*$4+$5*$5+$6*$6)}' > zz1.dat
$AWK -v x=$lon -v y=$lat -v z=$alt '{print x, y, z, $1}' zz1.dat | gmt mgd77magref -Frt/923456 | awk '{print $4, $5}' > zz2.dat
# Compute data & model min/max
m1=($(gmt info zz1.dat -C -f0T))
m2=($(gmt info zz2.dat -C -f0T))

# Compute limits such that both Y axes have the same scale
max_Y=$(echo ${m1[2]} ${m1[3]} ${m2[2]} ${m2[3]} | $AWK '{if (($2-$1) > ($4-$3)) print($2-$1); else print($4-$3)}')
y1_max=$(echo ${m1[2]} $max_Y | $AWK '{print $1 + $2}')
y2_max=$(echo ${m2[2]} $max_Y | $AWK '{print $1 + $2}')
gmt psxy zz1.dat -R${m1[0]}/${m1[1]}/${m1[2]}/$y1_max -JX16c/6c -Bpxa6Hf1h -By10 -BWSn -W1p -Y5.0c -P -K > $ps
gmt psxy zz2.dat -R${m2[0]}/${m2[1]}/${m2[2]}/$y2_max -J -Bpxa6Hf1h -By10 -BE -W1p,red --MAP_DEFAULT_PEN=+1p,red -O -K >> $ps

gmt math zz1.dat zz2.dat SUB -o1 -f0T = dif_T.dat
std=$(gmt math dif_T.dat STD -S = | $AWK '{printf "%.2f\n", $1}')
mean=$(gmt math dif_T.dat MEAN -S = | $AWK '{printf "%.2f\n", $1}')

# Write Date, MEAN & STD
t=($(echo ${m2[2]} | $AWK '{print $1 + 4, $1 + 7, $1+12}'))
echo ${m1[0]} ${t[1]} Mean = $mean | gmt pstext -F+f11p,Bookman-Demi+jLB -R -J -N -X0.5c -O -K >> $ps
echo ${m1[0]} ${t[0]} STD = $std | gmt pstext -F+f11p,Bookman-Demi+jLB -R -J -N -O -K >> $ps
echo ${m1[0]} ${t[2]} $data | gmt pstext -F+f12p,Bookman-Demi+jLB -R -J -N -O -K >> $ps
station=$(tail -n +4 $dia | head -1 | $AWK '{print $3}')
echo ${m1[0]} ${t[0]} Station -- $station | gmt pstext -F+f14p,Bookman-Demi+jLB -R -J -N -Xa7.5c -Ya4.7c -O -K >> $ps

# Compute and write the IGRF for this day
IGRF=$(echo $lon $lat $alt $data | gmt mgd77magref -Ft/0 | $AWK '{printf "%.2f\n", $1}')
echo ${m1[0]} ${t[0]} IGRF = $IGRF | gmt pstext -F+f15p,Bookman-Demi+jCT -R -J -N -Xa7.5c -Ya3.0c -O -K >> $ps

# Plot histogram of differences with mean removed
gmt math dif_T.dat $mean SUB = | gmt pshistogram -F -W2 -G0 -JX4c/3c -Bx5 -By100 -BWN -Xa11.5c -O >> $ps

