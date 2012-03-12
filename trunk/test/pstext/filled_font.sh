#!/bin/bash
# Test pstext with filled and outline fonts

. functions.sh
header "Test plotting filled and outline fonts"

ln -fs ${GMT_SOURCE_DIR}/share/psldemo/circuit.ras .

psbasemap -R0/18/0/13 -Jx1c -B5g1WSne -P -K > $ps
( awk '{print 1,13-NR*2,$1,$1}' | pstext -R -J --FONT=48p,Helvetica-Bold -F+f+jBL -O) >> $ps << EOF
red
red=1p,blue
p150/25:BredFblue
-=1p,blue
p150/circuit.ras
p150/7=0.25p
EOF

pscmp
