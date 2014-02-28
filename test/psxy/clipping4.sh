#!/bin/bash
#       $Id$
#
# Check ellipse clipping and filling/outline

ps=clipping4.ps

cat << EOF > t.txt
170	78	4000
190	-78	4000
180	0	5000
EOF

gmt psxy -R0/360/-75/75 -JM7i -P -Baf -BWSne -Xc -SE- -Gred -W10p,blue t.txt > $ps

# To create the desired plot I had to fake it this way to make sure the above will fail
#dy=`echo 0 -75 | mapproject -R0/360/-77/77 -JM7i -Di -o1`
#gmt psclip -R0/360/-75/75 -JM7i -P -Xc -T -K > $ps
#gmt psxy -R0/360/-77/77 -J -O -K -SE- -Gred -W10p,blue t.txt -Y-${dy}i >> $ps
#gmt psclip -R -J -O -K -C >> $ps
#gmt psbasemap -R0/360/-75/75 -J -O -Baf -BWSne -Y${dy}i >> $ps
