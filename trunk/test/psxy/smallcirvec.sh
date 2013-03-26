#!/bin/bash
#       $Id$
#
# Check geovector symbols for small circle vectors

ps=smallcirclvec.ps

# Vector from point with head at end
cat << EOF > t.txt
0 0 5000
EOF
psxy -R-180/180/-75/75 -JM6i -Bag -W0.5p,red -S=0.4i+jb+e+r+o0/45 -P -Gred --MAP_VECTOR_SHAPE=1 t.txt -K -Xc > $ps
psxy -R -J -O -K -Sc0.05i -Gred t.txt >> $ps
psxy -R -J -O -T >> $ps
