#!/bin/bash
#       $Id$
#
# Check geovector symbols

. ../functions.sh
header "Test psxy with geovector symbols"
ps=geovector.ps

cat << EOF > t.txt
-40 30 60 19000
0 -30 20 13000
EOF

psxy -R-180/180/-75/75 -JM6i -Bag10 -W0.5p,red -S=0.3i+jb+e -P -Gred --MAP_VECTOR_SHAPE=1 t.txt -K > $ps
psxy -R -J -O -K -Sc0.05i -Gred t.txt >> $ps
cat << EOF > t.txt
-120 0 45 10000
60 -50 20 5000
EOF
psxy -R -J -W0.5p,blue -S=0.3i+jc+b+e -Gblue --MAP_VECTOR_SHAPE=1 t.txt -O -K >> $ps
psxy -R -J -Sc0.05i -Gblue t.txt -O >> $ps

rm -f t.txt
pscmp
