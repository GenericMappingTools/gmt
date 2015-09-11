#!/bin/bash
#       $Id$

# Testing gmt math for BCDF PCDF ZCDF TCDF FCDF CHICDF
# Red dots are output from MATLAB for comparison
ps=cdfs.ps
# Plot binomial cumulative distribution
gmt math -T0/8/1 0.25 8 T BCDF = p.d
cat << EOF > ML.txt
0	0.100112915039063
1	0.367080688476563
2	0.678543090820313
3	0.886184692382812
4	0.972702026367188
5	0.995773315429688
6	0.999618530273438
7	0.999984741210938
8	1.000000000000000
EOF
gmt psxy -R-0.6/8.6/0/1 -JX6i/1.2i -P -K -Glightgreen p.d -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -Xc -Y0.75i --MAP_FRAME_TYPE=graph > $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Binomial P@-8,0.25@-" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTL+jTL -Dj0.1i/0 >> $ps
# Plot Poisson cumulative distribution
cat << EOF > ML.txt
0	0.135335283236613
1	0.406005849709838
2	0.676676416183063
3	0.857123460498547
4	0.947346982656289
5	0.983436391519386
6	0.995466194473751
7	0.998903281032141
8	0.999762552671739
EOF
gmt math -T0/8/1 T 2 PCDF = p.d
gmt psxy -R-0.6/8.6/0/1 -J -O -K -Glightgreen p.d -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -Y1.65i --MAP_FRAME_TYPE=graph >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Poisson P(@~l=2@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTL+jTL -Dj0.1i/0 >> $ps
# Plot normal cumulative distribution
cat << EOF > ML.txt
-4	0.000031671241833
-3	0.001349898031630
-2	0.022750131948179
-1	0.158655253931457
0	0.500000000000000
1	0.841344746068543
2	0.977249868051821
3	0.998650101968370
4	0.999968328758167
EOF
gmt math -T-4/4/0.1 T ZCDF = p.d
gmt psxy -R-4/4/0/1 -J -O -K p.d -W1p -BWS -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "N(0,1)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTL+jTL -Dj0.1i/0 >> $ps
# Plot t cumulative distribution
cat << EOF > ML.txt
-4	0.008065044950046
-3	0.019970984035859
-2	0.058058261758408
-1	0.186950483150030
0	0.500000000000000
1	0.813049516849971
2	0.941941738241592
3	0.980029015964141
4	0.991934955049954
EOF
gmt psxy -R-4/4/0/1 -J -O -K p.d -W1p,lightgray -BWS -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt math -T-4/4/0.1 T 4 TCDF = p.d
gmt psxy -R -J -O -K p.d -W1p >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "t(@~n=4@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTL+jTL -Dj0.1i/0 >> $ps
# Plot F cumulative distribution
cat << EOF > ML.txt
0	                0
1	0.482684447966620
2	0.890651453271511
3	0.972605894491473
4	0.991413968598696
5	0.996792747099576
6	0.998631115704947
7	0.999352365977181
8	0.999667605378123
EOF
gmt math -T0/8/0.02 T 20 12 FCDF = p.d
gmt psxy -R0/8/0/1 -J -O -K p.d -W1p -BWS -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "F(@~n@-1@-=20, n@-2@- = 12@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTL+jTL -Dj0.1i/0 >> $ps
# Plot Chi^2 cumulative distribution
cat << EOF > ML.txt
0	                0
1	0.090204010431050
2	0.264241117657115
3	0.442174599628926
4	0.593994150290162
5	0.712702504816354
6	0.800851726528544
7	0.864111774599567
8	0.908421805556329
EOF
gmt math -T0/8/0.1 T 4 CHICDF = p.d
gmt psxy -R0/8/0/1 -J -O -K p.d -W1p -BWS+t"Cumulative Distribution Functions" -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "@~c@~@+2@+(@~n=4@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTL+jTL -Dj0.1i/0 >> $ps
# Done
gmt psxy -R -J -O -T >> $ps
