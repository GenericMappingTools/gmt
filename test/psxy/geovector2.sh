#!/bin/bash
#
# Check geovector symbols

ps=geovector2.ps

# Vector from point with head at end
cat << EOF > t.txt
-40 30 60 10000n
0 -30 20 8000n
EOF
gmt psxy -R-180/180/-75/75 -JM6i -Bag -W0.5p,red -S=0.4i+jb+e+r+h1 -P -Gred t.txt -K -Xc > $ps
gmt psxy -R -J -O -K -Sc0.05i -Gred t.txt >> $ps
# Centered double-head vector
cat << EOF > t.txt
-120 0 45 100d
60 -50 20 60d
EOF
gmt psxy -R -J -W0.5p,blue -S=0.4i+jc+b+e+h1 -Gblue t.txt -O -K >> $ps
gmt psxy -R -J -Sc0.05i -Gblue t.txt -O -K >> $ps
# Vector to point with head at start
cat << EOF > t.txt
-60 10 5 8000
-140 -65 80 6000
EOF
gmt psxy -R -J -W0.5p,black -S=0.4i+je+e+a20+l+h1 t.txt -O -K >> $ps
gmt psxy -R -J -Sc0.05i -Gblack t.txt -O >> $ps

