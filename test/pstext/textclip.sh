#!/bin/bash
# $Id: textclip.sh,v 1.2 2011-03-22 18:41:32 guru Exp $
# Test pstext with text path clipping
. ../functions.sh
ps=textclip.ps
header "Test text clip paths and delayed annotations"
FONT=16p,Helvetica

psbasemap -R0/10/0/10 -JX15c/10c -Ggray70 -K -P > $ps

pstext -R -J -F+f${FONT}+jBL -Gc -C0 -O -K << EOF >> $ps
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
