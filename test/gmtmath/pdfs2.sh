#!/bin/bash
#       $Id$

# Testing gmt math for LPDF EPDF RPDF
# Red dots are output from MATLAB (if available) for comparison
ps=pdfs2.ps
# Plot Laplace distribution
gmt math -T-4/4/0.1 T LPDF = p.d
cat << EOF > ML.txt
EOF
gmt psxy -R-4/4/0/0.5 -JX6i/1.2i -P -K p.d -W1p -BWS -Bxa1 -Byaf -Xc -Y0.75i --MAP_FRAME_TYPE=graph > $ps
#gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Laplace P(z)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot Exponential distribution
cat << EOF > ML.txt
0	2.000000000000000
1	0.270670566473225
2	0.036631277777468
3	0.004957504353333
4	0.000670925255805
5	0.000090799859525
6	0.000012288424707
7	0.000001663057438
8	0.000000225070349
EOF
gmt math -T0/8/0.1 T 2 EPDF = p.d
gmt psxy -R0/8/0/2.5 -J -O -K p.d -W1p -BWS -Bxa1 -Byaf -Y1.65i --MAP_FRAME_TYPE=graph >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Exponential P(@~l=2@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot Rayleigh distribution
cat << EOF > ML.txt
0	                0
1	0.606530659712633
2	0.270670566473225
3	0.033326989614727
4	0.001341850511610
5	0.000018633265860
6	0.000000091379878
7	0.000000000160281
8	0.000000000000101
EOF
gmt math -T0/8/0.1 T RPDF = p.d
gmt psxy -R0/8/0/0.8 -J -O -K p.d -W1p -BWS -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Rayleigh(z)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot Weibull distribution
cat << EOF > ML.txt
0	                0
1	0.551819161757164
2	0.125382222603158
3	0.014387706241539
4	0.001006387883708
5	0.000046775274225
6	0.000001521835153
7	0.000000035924763
8	0.000000000631937
EOF
gmt math -T0/8/0.1 T 1 1.5 WPDF = p.d
gmt psxy -R0/8/0/0.8 -J -O -K p.d -W1p -BWS -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "Weibull(z)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Plot CHIDIST distribution
cat << EOF > ML.txt
0	                0
1	0.151632664928158
2	0.183939720585721
3	0.167347620111322
4	0.135335283236613
5	0.102606248279873
6	0.074680602551796
7	0.052845420989057
8	0.036631277777468
EOF
gmt math -T0/8/0.1 T 4 CHIPDF = p.d
gmt psxy -R0/8/0/0.25 -J -O -K p.d -W1p -BWS+t"Probability Density Functions" -Bxa1 -Byaf --MAP_FRAME_TYPE=graph -Y1.65i >> $ps
gmt psxy -R -J -O -K ML.txt -Sc0.2c -Gred -N >> $ps
echo "@~c@~@+2@+(@~n=4@~)" | gmt pstext -R -J -O -K -F+f12p,Times-Italic+cTR+jTR >> $ps
# Done
gmt psxy -R -J -O -T >> $ps
