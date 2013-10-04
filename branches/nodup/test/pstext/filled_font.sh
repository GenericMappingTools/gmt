#!/bin/bash
# Test gmt pstext with filled and outline fonts

ps=filled_font.ps

ln -fs ${GMT_SOURCE_DIR}/share/psldemo/circuit.ras .

gmt psbasemap -R0/18/0/13 -Jx1c -B5g1 -BWSne -P -K > $ps
( $AWK '{print 1,13-NR*2,$1,$1}' | gmt pstext -R -J --FONT=48p,Helvetica-Bold -F+f+jBL -O) >> $ps << EOF
red
red=1p,blue
p150/25:BredFblue
-=1p,blue
p150/circuit.ras
p150/7=0.25p
EOF

