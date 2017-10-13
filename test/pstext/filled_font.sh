#!/bin/bash
# $Id$
# Test gmt pstext with filled and outline fonts

ps=filled_font.ps

gmt psbasemap -R0/18/0/15 -Jx1c -B5g1 -BWSne -P -K > $ps
( $AWK '{print 1,15-NR*2,$1,$1}' | gmt pstext -R -J --FONT=48p,Helvetica-Bold -F+f+jBL -O) >> $ps << EOF
red
red=3p,green
red=~3p,green
p25+r150:BredFblue
-,-,-=3p,blue
p@circuit.png+r150
p7+r150=0.25p
EOF
