#!/usr/bin/env bash
ps=matharcvar.ps
cat << EOF > matharc-const.def
0 0 1 y
0 0 0.3 0 60 m
EOF
cat << EOF > matharc-two.def
N: 2 oo
0 0 1 y
0 0 0.3 \$1 \$2 m
EOF
cat << EOF > matharc-one.def
N: 1 o
0 0 1 y
0 0 0.3 0 \$1 m
EOF
echo "0 0 60 fixed" > q1.txt
echo "0 0 90 one" > q2.txt
echo "0 0 140 one " > q3.txt
echo "0 0 20 270 two" > q4.txt
gmt psxy -R-0.7/0.7/-0.7/0.7 -JX7c -Skmatharc-one/5c -P -N -W2p,red -Gred -Bag0.5 -BWSne q3.txt -K -X1.25i > $ps
awk '{printf "@~a@~ = %s\n", $3}' q3.txt | gmt pstext -R -J -O -K -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
gmt psxy -R -J -Skmatharc-two/5c -O -N -W2p,red -Gred -Bag0.5 -BWSne q4.txt -K -Y10c >> $ps
awk '{printf "@~a@~ = %s-%s\n", $3, $4}' q4.txt | gmt pstext -R -J -O -K -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
gmt psxy -R -J -Skmatharc-const/5c -O -N -W2p,red -Gred -Bag0.5 -BWSne q1.txt -K -X9c >> $ps
awk '{printf "@~a@~ = %s\n", $3}' q1.txt | gmt pstext -R -J -O -K -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
gmt psxy -R -J -Skmatharc-one/5c -O -N -W2p,red -Gred -Bag0.5 -BWSne q2.txt -K -Y-10c >> $ps
awk '{printf "@~a@~ = %s\n", $3}' q2.txt | gmt pstext -R -J -O -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
