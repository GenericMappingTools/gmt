#!/usr/bin/env bash
#
# Test psxyz -Zfile for coloring

ps=variable_z.ps

cat << EOF > pol.txt
>
0 0 0
0 1 0
1 1 0
1 0.3 0
0 0 0
>
1 1 0
1 2 0
2 2 0
2 1 0
1 1 0
EOF
cat << EOF > z.txt
1
4
EOF
gmt makecpt -Chot -T0/5 > t.cpt

gmt psxyz -R-2/5/-1/3/0/1 -Jx2c -Jz1c pol.txt -Zz.txt -B0 -Ct.cpt -W1p -P -K -X4c -Y1.5c -p135/35 > $ps
gmt psxyz -R -J -Jz pol.txt -Zz.txt+f -B0 -Ct.cpt -W1p -p -O -K -Y8c >> $ps
gmt psxyz -R -J -Jz pol.txt -Zz.txt+l -B0 -Ct.cpt -W1p -p -O -Y8c >> $ps
