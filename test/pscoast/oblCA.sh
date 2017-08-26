#!/bin/bash
# This is an oblique Mercator map of CA.  The user Meridith Kraner (mkraner@nevada.unr.edu)
# wants this plot to be aligned so CA long-axis is approximately N-S.  This would mean specifying
# the LL and UR using the red dots.  However, this gives junk.  Selecting the opposite
# corners (green) works as originally intended but here we want the opposite, i.e., a 90 rotation.
# Currently does not work in GMT.  Need to debug to determine why red points do not work.
# No PS added so test will fail until fixed.
# P. Wessel, Aug 17, 2017.

ps=oblCA.ps

Rbox=127W/114W/33/39
#LR and UL -> Doesn't follow directions, but gives a pretty map with upside down latitudes
PROJ1="-JOc-120.3938/36.1433/-70/51/6.0i  -R-117.84865/35.59988/-122.94715/36.71517r"

#LL and UR -> FOLLOWS THE DIRECTIONS, BUT DOESN'T WORK FOR SOME REASON
PROJ2="-JOc-120.3938/36.1433/-70/51/8.0i -R-120.01540/33.83507/-120.76423/38.40270r"

gmt pscoast -R$Rbox -JM6i -Slightblue -W0.25p -Baf -K -P -Na/.5p,gray -Xc > $ps
gmt psbasemap $PROJ1 -A | gmt psxy -R$Rbox -JM6i -W1p,red -O -K >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Ggreen << EOF >> $ps
-117.84865	35.59988
-122.94715	36.71517
EOF
gmt psxy -R -J -O -K -Sc0.1i -Gred << EOF >> $ps
-120.01540	33.83507
-120.76423	38.40270
EOF
gmt pscoast $PROJ1 -B1g1 -W.75p -Na/.5,80/80/80 -Slightblue -Df -Clightblue -Ir/.24p,lightblue -O -Y4.5i --MAP_ANNOT_OBLIQUE=0 >> $ps
