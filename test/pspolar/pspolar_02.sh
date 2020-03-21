#!/usr/bin/env bash

# Draw fault planes and points that should lie on them
# K. Feigl, 2015-11-08

ps=pspolar_02.ps

gmt psbasemap -R-8/8/-10/10 -JX7i -P -Xc -K -B+t"circles should plot on curves" > $ps

COUNTER=0
while [  $COUNTER -lt 10 ]; do
     let COUNTER=COUNTER+1
     let STRIKEF=`echo $COUNTER | awk '{print $1*36}'`  # strike of fault plane
     let STRIKEP=`echo $STRIKEF | awk '{print $1+90}'`  # strike of point
     let DIP=`   echo $COUNTER | awk '{print $1*9}'`
     #echo COUNTER $COUNTER STRIKE $STRIKEF DIP $DIP

# plot planes
gmt psmeca -R -J -P -M -Sa6i -N -W1p -O -K -h4 -C -T << EOF >> $ps
#a) Focal mechanism in Aki & Richard's convention:
#  0   1    2       3     4      5  6      7     8       9
#  X,  Y, depth, strike,   dip, rake, mag, newX, newY, event_title
#  0     0.   1.        0    90     0. 5.5   0.   0.   test90-90
  0     0.   1.   $STRIKEF $DIP     0. 5.5   0.   0.   $COUNTER
EOF

# and polarities observed
gmt pspolar -R -J -D0./0. -M4c -N -Sc0.2i -Qe -O -K -P -h3 << EOF >> $ps
# ID strike plunge Compression/Dilatation
#  1 90 45 d
#  1  0  45 d
1 $STRIKEP $DIP d
EOF

done

gmt psbasemap -R -J -P -V -O -B0 >> $ps
