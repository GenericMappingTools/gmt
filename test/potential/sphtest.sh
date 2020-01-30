#!/usr/bin/env bash
#
# Computes the gravity and VGG anomaly over a sphere and compares
# with theory a
ps=sphtest.ps
# sphere_mod.txt was made thus:
# sphere 2000 50 50 4000
R=2000
z0=4000
gmt math -T-25/25/0.2 0 = trk
gmt talwani3d @sphere_mod.txt -D1670 -Mh -Ntrk -o0,3 > faa.txt
gmt talwani3d @sphere_mod.txt -D1670 -Mh -Ntrk -o0,3 -Fv > vgg.txt
gmt talwani3d @sphere_mod.txt -D1670 -Mh -Ntrk -o0,3 -Fn > n.txt
gmt psbasemap -R-25/25/-5/120 -JX6i/6i -P -K -Xc -Y4i -Bxafg1000 -Byafg1000+l"mGal or Eotvos" -BWsn+t"Testing FAA, VGG and Geoid over sphere" > $ps
cg=$(gmt math -Q 1.0e5 4.0 MUL PI MUL 6.673e-11 MUL $R 3 POW MUL $z0 MUL 1670.0 MUL 3.0 DIV =)
cv=$(gmt math -Q 1.0e9 4.0 MUL PI MUL 6.673e-11 MUL $R 3 POW MUL 1670.0 MUL 3.0 DIV =)
cn=$(gmt math -Q 4.0 3.0 DIV PI MUL $R 3 POW MUL 1670.0 MUL 6.673e-11 MUL =)
gmt math -T-25/25/0.1 $cg $z0 T 1000 MUL HYPOT 3 POW DIV = s_g.txt
gmt math -T-25/25/0.1 $cv $z0 T 1000 MUL R2 $z0 2 POW 3 MUL SUB $z0 T 1000 MUL HYPOT 5 POW DIV MUL NEG = s_v.txt
gmt math -T-25/25/0.1 $cn $z0 T 1000 MUL HYPOT DIV 9.81 DIV = s_n.txt
gmt psxy -R -J -W0.25p,red -O -K s_g.txt >> $ps
gmt psxy -R -J -W0.25p,blue -O -K s_v.txt >> $ps
gmt psxy -R -J faa.txt -Sc0.02i -Gred -O -K  >> $ps
gmt psxy -R -J -O -K vgg.txt -Sc0.02i -Gdarkgreen >> $ps
gmt psxy -R-25/25/0/0.15 -J -O -K s_n.txt -W0.25p,brown -Byafg1000+l"m" -BE >> $ps
gmt psxy -R -J -O -K n.txt -Sc0.02i -Gbrown >> $ps
cat << EOF > legend.txt
S 0.2i - 0.3i - 0.25p,red 0.5i FAA (sphere)
S 0.2i - 0.3i - 0.25p,blue 0.5i VGG (sphere)
S 0.2i - 0.3i - 0.25p,brown 0.5i N (sphere)
S 0.2i c 0.1i red - 0.5i FAA (Talwani3D)
S 0.2i c 0.1i darkgreen - 0.5i VGG (Talwani3D)
S 0.2i c 0.1i brown - 0.5i N (Talwani3D)
EOF
gmt pslegend -R -J -O -K -DjTL+w2.2i+jTL+o0.1i/0.1i legend.txt -F+gwhite+p >> $ps
# Plot sphere
mx=$(gmt math -Q $R 2 MUL 1000 DIV 50 DIV 6 MUL =)
mz=$(gmt math -Q $R 2 MUL 6000 DIV 2.5 MUL =)
gmt psxy -R-25/25/0/6000 -JX6i/-2.5i -O -K -Y-2.75i -Se -Gblack -Bxafg1000+u" km" -Byafg4000 -BWSne << EOF >> $ps
0 4000 0 ${mx}i ${mz}i
EOF
echo 0 1500 @~Dr = 1670@~ | gmt pstext -R -J -O -K -F+f18p,Helvetica+jCM -Gwhite >> $ps
echo -3 4000 R = $R m | gmt pstext -R -J -O -K -F+f14p,Times-Italic+jRB -Dj0.1i/0.1i >> $ps
echo 3 4000 z@-0@- = $z0 m | gmt pstext -R -J -O -K -F+f14p,Times-Italic+jLB -Dj0.1i/0.1i >> $ps
gmt psxy -R -J -O -T >> $ps
