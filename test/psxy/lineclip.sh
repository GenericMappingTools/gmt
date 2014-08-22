#!/bin/bash
# Example of problem submitted by Roger Davis, HMRG, Issue #603
# Point 0 is slightly inside, all others are outside. Got diagonal
# line from TR corner to point 3
ps=lineclip.ps
cat <<END > t.txt
-131.69539 9.882795
-131.60807 9.88348
-131.60749 9.80983
-131.69480 9.80915
END

gmt mapproject -R-131.74249/9.8087739/-131.65578/9.883111r -Ju9/1:90000 t.txt -Di > t.xy
gmt psxy -R-1/10/-1/10 -Jx1i t.xy -W0.25p,- -K -P -X0 -Y0 > $ps
gmt psxy -A -R-131.74249/9.8087739/-131.65578/9.883111r -Ju9/1:90000 -L t.txt -W3p -O -K -X1i -Y1i >> $ps
gmt psxy -R -J -O -K -N -Sc0.25i -Gyellow -W0.25p t.txt -Baf >> $ps
awk '{print $1, $2, NR-1}' t.txt | gmt pstext -R -J -O -K -N -F+f12p+jCM >> $ps

# Change first y coordinate from 9.882795 to 9.882794
cat <<END > t.txt
-131.69539 9.882794
-131.60807 9.88348
-131.60749 9.80983
-131.69480 9.80915
END

gmt mapproject -R-131.74249/9.8087739/-131.65578/9.883111r -Ju9/1:90000 t.txt -Di > t.xy
gmt psxy -R-1/10/-1/10 -Jx1i t.xy -W0.25p,- -K -O -Y4i -X-1i >> $ps
gmt psxy -A -R-131.74249/9.8087739/-131.65578/9.883111r -Ju9/1:90000 -L t.txt -W3p -O -K -X1i -Y1i >> $ps
gmt psxy -R -J -O -K -N -Sc0.25i -Gyellow -W0.25p t.txt -Baf >> $ps
awk '{print $1, $2, NR-1}' t.txt | gmt pstext -R -J -O -N -F+f12p+jCM >> $ps
