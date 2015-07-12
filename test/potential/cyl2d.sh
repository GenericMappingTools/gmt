#!/bin/bash
# Test talwani2d on 2-D horizontal cylinder
ps=cyl2d.ps
# Make a approximate cylinder with radius R = 1km and center depth 3 km
# Using density contrast of 1000 kg/m^3
# Compare with analytical expressions
R=1
Z=3
D=1000
dx=0.25
da=1
echo "> $D" > cyl.txt
gmt math -T0/360/$da -Ca T -C0 COSD $R MUL -C1 SIND $R MUL $Z ADD -Ca = >> cyl.txt
# Theoretical FAA:
gmt math -T-25/25/$dx T $Z R2 INV $Z MUL 1000 DIV 2 MUL PI MUL $R 1000 MUL 2 POW MUL 6.673e-11 MUL $D MUL 1e5 MUL = g_truth.txt
gmt talwani2d cyl.txt -T-25/25/$dx -Ff -Mhz -Z0 -V > faa.txt
gmt math -T-25/25/$dx T $Z R2 1000 2 POW MUL LOG 6.673e-11 MUL PI MUL $R 1000 MUL 2 POW MUL $D MUL 9.81 DIV NEG = n_truth.txt
gmt talwani2d cyl.txt -T-25/25/$dx -Fn -Mhz -Z0 -V > geoid.txt
gmt math -T-25/25/$dx T 2 POW $Z 2 POW SUB T $Z R2 2 POW DIV 2 MUL 6.673e-11 MUL PI MUL $R 1000 MUL 2 POW MUL $D MUL 1e3 MUL NEG = v_truth.txt
gmt talwani2d cyl.txt -T-25/25/$dx -Fv -Mhz -Z0 -V > vgg.txt

gmt psxy -R-25/25/0/5 -JX6i/-1.75i cyl.txt -Ggray -W1p -P -X1.5i -K -Bxaf+u" km" -Byaf+l"km" -BWSne > $ps
echo CYLINDER | gmt pstext -R -J -O -K -F+f14p+cTR+jTR -Dj0.1i >> $ps
gmt psxy -R-25/25/0/15 -JX6i/2.25i -O -K -Y1.9i g_truth.txt -Sc0.1c -Gred -Bxaf -Byaf+l"mGal" -BWsne >> $ps
gmt psxy -R -J -O -K faa.txt -W0.5p,blue >> $ps
echo FAA | gmt pstext -R -J -O -K -F+f14p+cTR+jTR -Dj0.1i >> $ps
gmt psxy -R-25/25/0/20 -JX6i/2.25i -O -K -Y2.4i n_truth.txt -Sc0.1c -Gred -Bxaf -Byaf+l"m" -BWsne >> $ps
gmt psxy -R -J -O -K geoid.txt -W0.5p,blue >> $ps
echo GEOID | gmt pstext -R -J -O -K -F+f14p+cTR+jTR -Dj0.1i >> $ps
gmt psxy -R-25/25/-10/50 -JX6i/2.25i -O -K -Y2.4i v_truth.txt -Sc0.1c -Gred -Bxaf -Byafg1000+l"Eotvos" -BWsne >> $ps
gmt psxy -R -J -O -K vgg.txt -W0.5p,blue >> $ps
echo VGG | gmt pstext -R -J -O -K -F+f14p+cTR+jTR -Dj0.1i >> $ps
gmt psxy -R -J -O -T >> $ps
gv $ps &
