#!/usr/bin/env bash
#
# Test psxy -Zfile for coloring

ps=variable_z.ps

cat << EOF > pol.txt
>
0 0
0 1
1 1
1 0.3
0 0
>
1 1
1 2
2 2
2 1
1 1
EOF
cat << EOF > z.txt
1
4
EOF
gmt makecpt -Chot -T0/5 > t.cpt

gmt psxy -R-2/5/-1/3 -Jx2c pol.txt -Zz.txt -B0 -Ct.cpt -W1p -P -K -X4c -Y1.5c > $ps
gmt psxy -R -J pol.txt -Zz.txt+f -B0 -Ct.cpt -W1p -O -K -Y8.5c >> $ps
gmt psxy -R -J pol.txt -Zz.txt+l -B0 -Ct.cpt -W1p -O -Y8.5c >> $ps
