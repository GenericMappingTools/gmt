#!/usr/bin/env bash
#
# Observatory magnetic data dl from ottawa.intermagnet.org/apps/dl_data_prel_e.php
# The first 30 lines of this file looks like this (so you understand the commands below):
#
# Format                 IAGA-2002                                    |
# Source of Data         Institut de Physique du Globe de Paris (IPGP)|
# Station Name           Chambon la Foret                             |
# IAGA CODE              CLF                                          |
# Geodetic Latitude      48.017                                       |
# Geodetic Longitude     2.266                                        |
# Elevation              145                                          |
# Reported               XYZF                                         |
# Sensor Orientation     HDZF                                         |
# Digital Sampling       5                                            |
# Data Interval Type     Average 1-Minute (00:30-01:29)               |
# Data Type              Definitive                                   |
# # D-conversion factor                                               |
# # K9-limit             500                                          |
# # This data file was converted from INTERMAGNET CD-ROM              |
# # Format binary data.                                               |
# # A complete set is available on the INTERMAGNET CD-ROM.            |
# # Go to www.intermagnet.org for details on obtaining this product.  |
# # CONDITIONS OF USE: These data are for scientific/academic use     |
# # For any other applications see the 'Conditions of Use' published  |
# # on the INTERMAGNET web site - www.intermagnet.org                 |
# # D conversion factor is a fixed value used to allow                |
# # Declination to be converted from minutes of arc to equivalent     |
# # nanoteslas. Set to H/3438*10000 where H is the annual mean        |
# # value of horizontal intensity.                                    |
#DATE       TIME         DOY     CLFX      CLFY      CLFZ      CLFF   |
#2001-05-01 00:00:00.000 121     20994.80   -617.30  42533.10  47436.50
#2001-05-01 00:01:00.000 121     20994.80   -617.10  42533.10  47436.50
#2001-05-01 00:02:00.000 121     20995.10   -617.00  42533.10  47436.70
#2001-05-01 00:03:00.000 121     20995.10   -617.00  42533.00  47436.50
#
# We need to pull out items such as lon,lat,altitude and the first time,
# which are lines 6,5,7 and 27, respectively.

ps=cm4.ps

gmt set FORMAT_DATE_MAP "o dd" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p
dia=$(gmt which -G @clf20010501d.min)

# Get Station location
lon=$(sed -n 6p $dia   | $AWK '{print $3}')
lat=$(sed -n 5p $dia   | $AWK '{print $3}')
alt=$(sed -n 7p $dia   | $AWK '{print $2/1000}')
time=$(sed -n 27p $dia | $AWK '{print $1}')

IGRF=$(echo $lon $lat $alt ${time}T | gmt mgd77magref -Fxyz/0)

# Create ISO datetime from date and time spread across two columns...
tail -n +27 $dia | $AWK '{print $1"T"$2, sqrt($4*$4+$5*$5+$6*$6)}' > zz1.dat
$AWK -v x=$lon -v y=$lat -v z=$alt '{print x, y, z, $1}' zz1.dat | gmt mgd77magref -Frt/923456 -o3,4 > zz2.dat
# Find data & model min/max
m1=($(gmt info zz1.dat -C -f0T))
m2=($(gmt info zz2.dat -C -f0T))

# Compute limits such that both Y axes have the same scale
max_Y=$(gmt math -Q ${m1[3]} ${m1[2]} SUB STO@D1 POP ${m2[3]} ${m2[2]} SUB STO@D2 POP @D2 @D1 GT @D2 @D1 IFELSE =)
y1_max=$(gmt math -Q ${m1[2]} $max_Y ADD =)
y2_max=$(gmt math -Q ${m2[2]} $max_Y ADD =)
gmt psxy zz1.dat -R${m1[0]}/${m1[1]}/${m1[2]}/$y1_max -JX16c/6c -Bpxa6Hf1h -By10 -BWSn -W1p -Y5.0c -P -K > $ps
gmt psxy zz2.dat -R${m2[0]}/${m2[1]}/${m2[2]}/$y2_max -J -Bpxa6Hf1h -By10 -BE -W1p,red --MAP_DEFAULT_PEN=+1p,red -O -K >> $ps

gmt math zz1.dat zz2.dat SUB -o1 -f0T = dif_T.dat
std=$(gmt math dif_T.dat STD -S = --FORMAT_FLOAT_OUT=%.2lf)
mean=$(gmt math dif_T.dat MEAN -S = --FORMAT_FLOAT_OUT=%.2lf)

# Write Date, MEAN & STD
t=($(echo ${m2[2]} | $AWK '{print $1 + 4, $1 + 7, $1+12}'))
echo ${m1[0]} ${t[1]} Mean = $mean | gmt pstext -F+f11p,Bookman-Demi+jLB -R -J -N -X0.5c -O -K >> $ps
echo ${m1[0]} ${t[0]} STD = $std | gmt pstext -F+f11p,Bookman-Demi+jLB -R -J -N -O -K >> $ps
echo ${m1[0]} ${t[2]} $time | gmt pstext -F+f12p,Bookman-Demi+jLB -R -J -N -O -K >> $ps
station=$(tail -n +4 $dia | head -1 | $AWK '{print $3}')
echo ${m1[0]} ${t[0]} Station -- $station | gmt pstext -F+f14p,Bookman-Demi+jLB -R -J -N -Xa7.5c -Ya4.7c -O -K >> $ps

# Compute and write the IGRF for this day
IGRF=$(echo $lon $lat $alt ${time}T | gmt mgd77magref -Ft/0 --FORMAT_FLOAT_OUT=%.2lf)
echo ${m1[0]} ${t[0]} IGRF = $IGRF | gmt pstext -F+f15p,Bookman-Demi+jCT -R -J -N -Xa7.5c -Ya3.0c -O -K >> $ps

# Plot histogram of differences with mean removed
gmt math dif_T.dat $mean SUB = | gmt pshistogram -F -W2 -G0 -JX4c/3c -Bx5 -By100 -BWN -Xa11.5c -O >> $ps
