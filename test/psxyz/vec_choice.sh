#!/usr/bin/env bash
# Demonstrate GMT 4 and GMT5+ vectors.  Derived from issue #1183
ps=vec_choice.ps
cat << EOF > t.txt
-107.059013 34.266248 0 0.0063771 157.8143 2i
-106.982134 34.253915 0 0.0058731 174.0829 2i
-107.000000 34.000000 0 0.0068731  74.0829 1i
EOF
gmt makecpt -Cjet -T0.005/0.007 > t.cpt
gmt psxyz -R-107:30/-106:30/33:30/34:30 -JM3i -P -Baf -BWSne -Sx0.1i -W0.5p t.txt -K > $ps
gmt psxyz -R -J -O -K -SV0.2i+e -W2.16p+c -Ct.cpt t.txt >> $ps
gmt pstext -R -J -O -K -F+f12p+cBL+t"-SV0.12i+e+a90 -W2.16p+cf -Ct.cpt" -Dj0.1i >> $ps
gmt pstext -R -J -O -K -F+f16p+cTL+t"GMT5-6" -Dj0.1i >> $ps
# GMT5 syntax
gmt psxyz -R -J -O -Baf -BWSne -Sx0.1i -W0.5p t.txt -K -X3.75i >> $ps
gmt psxyz -R -J -O -K -SV0.2i+bc -W2.16p+c -Ct.cpt t.txt >> $ps
gmt pstext -R -J -O -K -F+f12p+cBL+t"-SV0.2i+bc -W2.16p+c -Ct.cpt" -Dj0.1i >> $ps
gmt pstext -R -J -O -K -F+f16p+cTL+t"GMT5-6" -Dj0.1i >> $ps
gmt psxyz -R -J -O -Baf -BWSne -Sx0.1i -W0.5p t.txt -K -X-3.75i -Y4.25i >> $ps
gmt psxyz -R -J -O -K -SV0.03i/0.12i/0.1i -W0.25p -Ct.cpt t.txt >> $ps
gmt pstext -R -J -O -K -F+f12p+cBL+t"-SV0.03i/0.12i/0.1i -W0.25p -Ct.cpt" -Dj0.1i >> $ps
gmt pstext -R -J -O -K -F+f16p+cTL+t"GMT2-6" -Dj0.1i >> $ps
gmt psxyz -R -J -O -Baf -BWSne -Sx0.1i -W0.5p t.txt -K -X3.75i >> $ps
gmt psxyz -R -J -O -K -SV0.12i+e+a90 -W2.16p+cf -Ct.cpt t.txt >> $ps
gmt pstext -R -J -O -K -F+f12p+cBL+t"-SV0.12i+e+a90 -W2.16p+cf -Ct.cpt" -Dj0.1i >> $ps
gmt pstext -R -J -O -F+f16p+cTL+t"GMT5-6" -Dj0.1i >> $ps
