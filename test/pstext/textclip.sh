#!/bin/bash
# $Id$
# Test pstext with text path clipping

. ./functions.sh
header "Test text clip paths and delayed annotations"
FONT=16p,Helvetica

pstext -R0/10/0/10 -JX15c/10c -F+f${FONT}+jBL -Gc -B+ggray70 -C0 -K << EOF > $ps
1 3 This text should NOT be covered up by the box
1 8 Neither should this jolly text message
EOF

# Overlay big polygon
psxy -R -J -L -Gkhaki1 -W0.5p -O -K << EOF >> $ps
3 1
9 1
9 9
3 9
EOF

psclip -Cs -O >> $ps

pscmp
