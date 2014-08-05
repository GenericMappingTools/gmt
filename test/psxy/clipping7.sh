#!/bin/bash
ps=clipping7.ps
cat << EOF > badpol.txt
210 -46
210 -51
200 -51
200 -46
210 -46
EOF
scl=`gmtmath -Q 6 215 170 SUB DIV =`
x=`gmtmath -Q 205 170 SUB $scl MUL =`
psxy -R0/8/0/10 -Jx1i -P -K -W0.25p,- << EOF > $ps
$x	0
$x	8.5
EOF
psxy badpol.txt -R170/205/-52/-45 -Jm${scl}i -Gred -Baf -BWSne -O -A -K >> $ps
psxy badpol.txt -R170/215/-52/-45 -Jm -Gred -Baf -BWSne -O -K -A -Y2.25i >> $ps
psxy badpol.txt -R170/205/-52/-45 -Jm -Gred -Baf -BWSne -O -K -Y2.25i >> $ps
psxy badpol.txt -R170/215/-52/-45 -Jm -Gred -Baf -BWSne -O -Y2.25i >> $ps
gv $ps &
