#!/bin/bash
#       $Id$
#
# Check geovector symbols

ps=geovector.ps

# Vector from point with head at end
cat << EOF > t.txt
-40 30 60 19000
0 -30 20 13000
EOF
psxy -R-180/180/-75/75 -JM6i -Bag -W0.5p,red -S=0.4i+jb+e+r -P -Gred --MAP_VECTOR_SHAPE=1 t.txt -K -Xc > $ps
psxy -R -J -O -K -Sc0.05i -Gred t.txt >> $ps
# Centered double-head vector
cat << EOF > t.txt
-120 0 45 10000
60 -50 20 5000
EOF
psxy -R -J -W0.5p,blue -S=0.4i+jc+b+e -Gblue --MAP_VECTOR_SHAPE=1 t.txt -O -K >> $ps
psxy -R -J -Sc0.05i -Gblue t.txt -O -K >> $ps
# Vector to point with head at start
cat << EOF > t.txt
-60 10 5 8000
-140 -65 80 6000
EOF
psxy -R -J -W0.5p,black -S=0.4i+je+e+a20+l --MAP_VECTOR_SHAPE=1 t.txt -O -K >> $ps
psxy -R -J -Sc0.05i -Gblack t.txt -O >> $ps

