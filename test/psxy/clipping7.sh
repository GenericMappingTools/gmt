#!/bin/bash
ps=clipping7.ps
cat << EOF > badpol.txt
> -Gred
210 -46
210 -51
200 -51
200 -46
210 -46
> -Gblue
160 -46
160 -51
177 -51
177 -46
160 -46
EOF
scl=`gmt math -Q 6 215 170 SUB DIV =`
scl=`gmt math -Q 6 215 155 SUB DIV =`
xr=`gmt math -Q 205 155 SUB $scl MUL =`
xl=`gmt math -Q 170 155 SUB $scl MUL =`
gmt psxy -R0/8/0/10 -Jx1i -P -K -W0.25p,- << EOF > $ps
>
$xr	0
$xr	8.25
>
$xl	0
$xl	8.25
EOF
gmt psxy badpol.txt -R170/205/-52/-45 -Jm${scl}i -Gred -Baf -BWSne -O -A -K -X${xl}i >> $ps
gmt psxy badpol.txt -R155/215/-52/-45 -Jm -Gred -Baf -BWSne -O -K -A -Y2.25i -X-${xl}i >> $ps
gmt psxy badpol.txt -R170/205/-52/-45 -Jm -Gred -Baf -BWSne -O -K -Y2.25i -X${xl}i >> $ps
gmt psxy badpol.txt -R155/215/-52/-45 -Jm -Gred -Baf -BWSne -O -Y2.25i -X-${xl}i >> $ps
