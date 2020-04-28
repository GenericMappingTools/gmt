#!/usr/bin/env bash
#
# Test psxy -Zfile for coloring

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

gmt psxy -R-1/3/-1/3 -Jx2c -Jz1c pol.txt -B0 -Ct.cpt -Zz.txt -G+z -P -K -p165/35 -X2c -Y1.5c > $ps
echo "-Zz.txt -G+z" | gmt pstext -R -J -O -K -F+f12p+cTL -Dj0.1i -p >> $ps
gmt psxy -R -J pol.txt -B0 -Ct.cpt -Zz.txt -G+z -W2p -O -K -X8.5c -p >> $ps
echo "-Zz.txt -G+z -W2p" | gmt pstext -R -J -O -K -F+f12p+cTL -Dj0.1i -p >> $ps
gmt psxy -R -J pol.txt -B0 -Ct.cpt -Zz.txt -G+z -W2p+z -O -K -X-8.5c -Y8.5c -p >> $ps
echo "-Zz.tx -G+z -W2p+z" | gmt pstext -R -J -O -K -F+f12p+cTL -Dj0.1i -p >> $ps
gmt psxy -R -J pol.txt -B0 -Ct.cpt -Zz.txt -Gcyan -W2p+z -O -K -X8.5c -p >> $ps
echo "-Zz.txt -Gcyan -W2p+z" | gmt pstext -R -J -O -K -F+f12p+cTL -Dj0.1i -p >> $ps
gmt psxy -R -J pol.txt -B0 -Ct.cpt -Zz.txt -W2p+z -O -K -X-8.5c -Y8.5c -p >> $ps
echo "-Zz.txt -W2p+z" | gmt pstext -R -J -O -K -F+f12p+cTL -Dj0.1i -p >> $ps
gmt psxy -R -J pol.txt -B0 -Ct.cpt -Z3 -G+z -W2p -O -K -X8.5c -p >> $ps
echo "-Z3 -G+z -W2p" | gmt pstext -R -J -O -F+f12p+cTL -Dj0.1i -p >> $ps
