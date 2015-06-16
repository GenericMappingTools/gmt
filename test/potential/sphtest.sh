#! /bin/bash
# $Id$
#
# Computes the gravity and VGG anomaly over a sphere and compares
# with theory a
ps=sphtest.ps
# sphere.mod was made thus:
# sphere 2000 50 50 4000
R=2000
z0=4000
gmt math -T-25/25/0.2 0 = trk
gmt talwani3d sphere.mod -D1670 -Mh -Ntrk -o0,2 > faa.txt
gmt talwani3d sphere.mod -D1670 -Mh -Ntrk -o0,2 -Fp > vgg_p.txt
gmt talwani3d sphere.mod -D1670 -Mh -Ntrk -o0,2 -Fv > vgg_s.txt
gmt psbasemap -R-25/25/-5/120 -JX6i/6i -P -K -Xc -Y4i -Bxafg1000 -Byafg1000 -BWsne+t"Testing FAA and VGG over sphere" > $ps
cg=`gmt math -Q 1.0e5 4.0 MUL PI MUL 6.673e-11 MUL $R 3 POW MUL $z0 MUL 1670.0 MUL 3.0 DIV =`
cv=`gmt math -Q 1.0e9 4.0 MUL PI MUL 6.673e-11 MUL $R 3 POW MUL 1670.0 MUL 3.0 DIV =`
gmt math -T-25/25/0.1 $cg $z0 T 1000 MUL HYPOT 3 POW DIV = s_g.txt
gmt math -T-25/25/0.1 $cv $z0 T 1000 MUL R2 $z0 2 POW 3 MUL SUB $z0 T 1000 MUL HYPOT 5 POW DIV MUL NEG = s_v.txt
gmt psxy -R -J -W0.25p,red -O -K s_g.txt >> $ps
gmt psxy -R -J -W0.25p,blue -O -K s_v.txt >> $ps
gmt psxy -R -J faa.txt -Sc0.02i -Gred -O -K  >> $ps
gmt psxy -R -J -O -K vgg_s.txt -Sc0.02i -Gdarkgreen >> $ps
gmt psxy -R -J -O -K vgg_p.txt -Sc0.02i -Gorange >> $ps
cat << EOF > legend.txt
S 0.2i - 0.3i - 0.25p,red 0.5i FAA (sphere)
S 0.2i - 0.3i - 0.25p,blue 0.5i VGG (sphere)
S 0.2i c 0.1i red - 0.5i FAA (Talwani)
S 0.2i c 0.1i darkgreen - 0.5i VGG (Talwani-SSK)
S 0.2i c 0.1i orange - 0.5i VGG (Talwani-PW)
EOF
gmt pslegend -R -J -O -K -DjTL/2.2i/TL/0.1i/0.1i legend.txt -F+gwhite+p >> $ps
# Plot sphere
mx=`gmt math -Q $R 2 MUL 1000 DIV 50 DIV 6 MUL =`
mz=`gmt math -Q $R 2 MUL 6000 DIV 2.5 MUL =`
gmt psxy -R-25/25/0/6000 -JX6i/-2.5i -O -K -Y-2.75i -Se -Gblack -Bxafg1000+u" km" -Byafg4000 -BWSne << EOF >> $ps
0 4000 0 ${mx}i ${mz}i
EOF
echo 0 1500 @~Dr = 1670@~ | gmt pstext -R -J -O -K -F+f18p,Helvetica+jCM -Gwhite >> $ps
echo -3 4000 R = $R m | gmt pstext -R -J -O -K -F+f14p,Times-Italic+jRB -Dj0.1i/0.1i >> $ps
echo 3 4000 z@-0@- = $z0 m | gmt pstext -R -J -O -K -F+f14p,Times-Italic+jLB -Dj0.1i/0.1i >> $ps
gmt psxy -R -J -O -T >> $ps
