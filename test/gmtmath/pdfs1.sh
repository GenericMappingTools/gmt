#!/bin/bash
#       $Id$

# Testing gmt math for BPDF PPDF ZPDF TPDF FPDF CHIPDF
# Red dots are output from MATLAB for comparison
ps=pdfs1.ps
# Plot binomial distribution
gmt math -T0/8/1 0.25 8 T BPDF = p.d
cat << EOF > ML.txt
0	0.100112915039063
1	0.266967773437500
2	0.311462402343750
3	0.207641601562500
4	0.086517333984375
5	0.023071289062500
6	0.003845214843750
7	0.000366210937500
8	0.000015258789063
EOF
gmt psxy -R-0.6/8.6/0/0.35 -JX6i/1.2i -P -K -Glightgreen p.d -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -Xc -Y0.75i --MAP_FRAME_TYPE=graph > $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Binomial P@-8,0.25@-" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot POISSON distribution
cat << EOF > ML.txt
0	0.135335283236613
1	0.270670566473225
2	0.270670566473225
3	0.180447044315484
4	0.090223522157742
5	0.036089408863097
6	0.012029802954366
7	0.003437086558390
8	0.000859271639598
EOF
gmt math -T0/8/1 T 2 PPDF = p.d
gmt psxy -R-0.6/8.6/0/0.3 -J -O -K -Glightgreen p.d -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -Y1.65i --MAP_FRAME_TYPE=graph >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Poisson P(@~l=2@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot normal distribution
cat << EOF > ML.txt
-4	0.000133830225765
-3	0.004431848411938
-2	0.053990966513188
-1	0.241970724519143
0	0.398942280401433
1	0.241970724519143
2	0.053990966513188
3	0.004431848411938
4	0.000133830225765
EOF
gmt math -T-4/4/0.1 T ZPDF = p.d
gmt psxy -R-4/4/0/0.4 -J -O -K p.d -W1p -BWS -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Normal P(z)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot t distribution
cat << EOF > ML.txt
-4	0.006708203932499
-3	0.019693498090837
-2	0.066291260736239
-1	0.214662525839980
0	0.375000000000000
1	0.214662525839980
2	0.066291260736239
3	0.019693498090837
4	0.006708203932499
EOF
gmt psxy -R-4/4/0/0.4 -J -O -K p.d -W1p,lightgray -BWS -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt math -T-4/4/0.1 T 4 TPDF = p.d
gmt psxy -R -J -O -K p.d -W1p >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Student t(@~n=4@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot FDIST distribution
cat << EOF > ML.txt
0	                0
1	0.759526816440646
2	0.164497471943726
3	0.034650867728210
4	0.009138553307854
5	0.002925394505934
6	0.001089233824630
7	0.000456381170775
8	0.000210037817810
EOF
gmt math -T0/8/0.02 T 20 12 FPDF = p.d
gmt psxy -R0/8/0/1 -J -O -K p.d -W1p -BWS+t"Probability Density Functions" -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "F(@~n@-1@-=20, n@-2@- = 12@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Done
gmt psxy -R -J -O -T >> $ps
