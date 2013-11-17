#!/bin/bash
#       $Id$
#
# Check ellipse clipping and filling/outline

ps=clipping4.ps

cat << EOF > t.txt
170	78	4000
190	-70	4000
180	0	5000
EOF

gmt psxy -R0/360/-75/75 -JM7i -P -Baf -BWSne -Xc -SE- -Gred -W10p,blue t.txt > $ps
