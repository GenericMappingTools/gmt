#!/bin/bash
#       $Id$
#
# Check geovector symbols

. ../functions.sh
header "Test psxyz with geovector symbols"
ps=geovector.ps

cat << EOF > t.txt
-40 30 0 60 19000
0 -30 0 20 13000
EOF

psxyz -R-180/180/-75/75 -JM6i -Bag10 -W0.5p,red -S=0.3i+jb+e -P -Gred --MAP_VECTOR_SHAPE=1 t.txt -K -p155/35 > $ps
psxyz -R -J -O -K -Sc0.05i -Gred t.txt -p155/35 >> $ps
cat << EOF > t.txt
-120 0 0 45 10000
60 -50 0 20 5000
EOF
psxyz -R -J -W0.5p,blue -S=0.3i+jc+b+e -Gblue --MAP_VECTOR_SHAPE=1 t.txt -O -K -p155/35 >> $ps
psxyz -R -J -Sc0.05i -Gblue t.txt -O -p155/35 >> $ps

rm -f t.txt
pscmp
