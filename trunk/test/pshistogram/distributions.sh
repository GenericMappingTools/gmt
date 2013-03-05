#!/bin/bash
ps=distributions.ps
# LL: Histogram in counts with 3 normal distribution curves based on L2, L1, and LMS statistsics
pshistogram -Baf/afWSne+glightblue v3206.t -R-6000/0/0/1300 -JX3i/2.0i -Gorange -L1p -Z0 -W200 -P -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -Y0.75i  > $ps
echo "0 1300 Counts" | pstext -R -J -O -K -Dj0.1i -F+jTR+f12p >> $ps
# LR: Same, but in percent 
pshistogram -Baf/af:,%:WSne+glightblue v3206.t -R-6000/0/0/20 -JX3i/2.0i -Gorange -L1p -Z1 -W200 -O -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -X3.75i >> $ps
echo "0 20 Percent" | pstext -R -J -O -K -Dj0.1i -F+jTR+f12p >> $ps
# ML: Same, but in log 1+counts
pshistogram -Baf/afWSne+glightblue v3206.t -R-6000/0/0/8 -JX3i/2.0i -Gorange -L1p -Z2 -W200 -O -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -X-3.75i -Y2.5i >> $ps
echo "0 8 log1p(counts)" | pstext -R -J -O -K -Dj0.1i -F+jTR+f12p >> $ps
# MR: Same, but in log 1+precent
pshistogram -Baf/afWSne+glightblue v3206.t -R-6000/0/0/3.5 -JX3i/2.0i -Gorange -L1p -Z3 -W200 -O -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -X3.75i >> $ps
echo "0 3.5 log1p(percent)" | pstext -R -J -O -K -Dj0.1i -F+jTR+f12p >> $ps
# TL: Same, but in log10 1+counts
pshistogram -Baf/afWSne+glightblue v3206.t -R-6000/0/0/3.5 -JX3i/2.0i -Gorange -L1p -Z4 -W200 -O -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -X-3.75i -Y2.5i >> $ps
echo "0 3.5 log101p(counts)" | pstext -R -J -O -K -Dj0.1i -F+jTR+f12p >> $ps
# TR: Same, but in log10 1+precent
pshistogram -Baf/afWSne+glightblue v3206.t -R-6000/0/0/1.5 -JX3i/2.0i -Gorange -L1p -Z5 -W200 -O -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -X3.75i >> $ps
echo "0 1.5 log101p(percent)" | pstext -R -J -O -K -Dj0.1i -F+jTR+f12p >> $ps
# L: Cumulative histogram
pshistogram -Baf/afg100:,%:WSne+glightblue v3206.t -R-6000/0/0/105 -JX3i/2.0i -Glightred -L1p -Z1 -Q -W200 -O -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -X-3.75i -Y2.5i >> $ps
# R: Cumulative histogram staircase
pshistogram -Baf/afg100:,%:WSne+glightblue v3206.t -R-6000/0/0/105 -JX3i/2.0i -S -L1p -Z1 -Q -W200 -O -K -N0+p1p,red -N1+p1p,green -N2+p1p,blue -X3.75i >> $ps
pslegend -D0/0/1.2i/BR/0.1i/0.1i -R -J -O -K -F2p -Gwhite << EOF >> $ps
S 0.3i - 0.45i - 1p,red 0.7i L@-2@-
S 0.3i - 0.45i - 1p,green 0.7i L@-1@-
S 0.3i - 0.45i - 1p,blue 0.7i LMS
EOF
psxy -R -J -O -T >> $ps
