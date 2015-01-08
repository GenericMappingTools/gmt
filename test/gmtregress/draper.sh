#!/bin/bash
#       $Id$
# Testing gmtregress on the data in Draper & Smith [1998]
# Applied Regression Analysis, 3rd Ed, Wiley.

# Cols 3 and 4 are weights (1/sigma)

ps=draper.ps
cat << EOF > draper.txt
# Table 1.1 in Draper & Smith, Applied Regression Analysis
35.3	10.98
29.7	11.13
30.8	12.51
58.8	8.40
61.4	9.27
71.3	8.73
74.4	6.36
76.7	8.50
70.7	7.82
57.5	9.14
46.4	8.24
28.9	12.19
28.1	11.88
39.1	9.57
46.8	10.94
48.5	9.58
59.3	10.09
70.0	8.11
70.0	6.83
74.5	8.88
72.1	7.68
58.1	8.47
44.6	8.86
33.4	10.36
28.6	11.08
EOF
# First plot data and basic LS fit with equation
txt=`gmt regress -Ey -N2 -Fxm -T25/80/55 draper.txt -T0 | awk '{printf "85 13 @!y\\\\303 = %.4f %.4f x\n", $17, $15}'`
gmt psbasemap -R20/85/6/13 -JX6i/4i -P -Xc -K -Baf -BWSne > $ps
gmt regress -Ey -N2 -Fxym draper.txt | awk '{printf "> error\n%s %s\n%s %s\n", $1, $2, $1, $3}' | psxy -R -J -O -K -W0.25p,red,- >> $ps
gmt psxy -R -J -O -K draper.txt -Sc0.2c -Gblue >> $ps
gmt regress -Ey -N2 -Fxm -T25/80/55 draper.txt | gmt psxy -R -J -O -K -W2p >> $ps
echo "$txt" | gmt pstext -R -J -O -K -F+jRT+f18p -Dj0.1i >> $ps
# Redo plot and basic LS fit but also show 95% confidence band [NOT IMPLEMENTED YET]
gmt psxy -R -J -O -K draper.txt -Sc0.2c -Gblue -Baf -BWSne+t"Draper & Smith [1998] Regression" -Y4.5i >> $ps
gmt regress -Ey -N2 -Fxm -T25/80/55 draper.txt | gmt psxy -R -J -O -K -W2p >> $ps
gmt psxy -R -J -O -T >> $ps
