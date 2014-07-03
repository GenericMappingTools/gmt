#!/bin/bash
# Check clipping of quoted lines
ps=ortho.ps
# station lat,long
LON_E='-57.93229950'
LAT_E='-34.90674529'
#
LAT_C='20'
LON_C='-40'
RADIUS='1000'

gmt psbasemap -Rg -JG$LON_C/$LAT_C/7i -Ba0f30g30 -K -Xc -P > $ps

# distances from north pole to station

rm -f lines.dat
touch lines.dat
gmt project -C$LON_E/90 -E$LON_E/-90 -G$RADIUS -Q > distances.xyp
for LD in `awk '{if ( ( $2 > 0 ) && ( $3 > 0 ) ) {print $2"/"$3}}' distances.xyp`
do
   LAT=`echo $LD | awk -F '/' '{print $1}'`
   DIST=`echo $LD | awk -F '/' '{print $2}'`
   echo "> -L$DIST" >> lines.dat
   LON='0'
   while [ $LON -le 360 ]
   do
      echo "$LON $LAT" >> lines.dat
      LON=$(($LON+1))
   done
done

gmt psxy lines.dat -R -J -Sqxdistances.xyp:+f8p,Helvetica,black+gwhite+Lh+u" km"+v -W0.5p,green -O >> $ps

