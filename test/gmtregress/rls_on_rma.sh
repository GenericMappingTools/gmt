#!/usr/bin/env bash
# Test Reweighted L-2 measure using Reduced Major Axis area misfit
# Make sure the Reweighted path (-Nw) yields the same regression as
# regression just the good points after removing the outliers.
# Test data from Dietmar Muller.
ps=rls_on_rma.ps

echo "0	red	1	red" > code.cpt
echo "1	blue	2	blue" >> code.cpt
gmt regress noisy_data.txt -Nr -Er > rma.txt
gmt regress noisy_data.txt -Nr -Er -S > raw.txt
gmt regress noisy_data.txt -Nw -Er > model.txt
gmt psxy rma.txt -R-200/-30/200/800 -JX6i/9i -P -Baf -W2p,green -i0,2 -K -Xc > $ps
gmt psxy model.txt -R -J -O -K -W4p -i0,2 >> $ps
gmt regress raw.txt -i0,1 -Er | gmt psxy -R -J -O -K -W1p,orange -i0,2 >> $ps
gmt psxy model.txt -i0,1,6 -Ccode.cpt -R -J -O -Sc0.25c -K >> $ps
gmt pslegend -R -J -O -DjBR+w3.5i+jBR+o0.1i -F+p1p << EOF >> $ps
S 0.2i c 0.25c blue - 0.5i Good point according to @;green;RMA via LMS@;;
S 0.2i c 0.25c red  - 0.5i Outlier according to @;green;RMA via LMS@;;
D 0.1i 0.25p
S 0.2i - 0.25i - 2p,green 0.5i RMA via LMS
S 0.2i - 0.25i - 4p 0.5i Reweighted RMA
S 0.2i - 0.25i - 1p,orange 0.5i RMA via LS on good points only
EOF
