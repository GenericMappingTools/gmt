#!/bin/bash
#	$Id$
#
# Show all splines and their derivatives for a basic data set
ps=splines.ps
cat << EOF > t.txt
0	0
1	1
2	1.5
3	1.25
4	1.5
4.5	3
5	2
6	2.5
EOF
# Splines
gmt sample1d t.txt -S0/6 -I0.01 -Fc | gmt psxy -R-0.1/6.1/-0.1/3.1 -JX6i/3i -P -Bafg -By+l"u(x)" -BWSne -W1p -K -X1.5i > $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fl | gmt psxy -R -J -O -K -W1p,blue >> $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fa | gmt psxy -R -J -O -K -W1p,red >> $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fn | gmt psxy -R -J -O -K -W1p,darkgreen >> $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gred -Wthin >> $ps
gmt pslegend -R -J -O -K -DjTL+w1.9i+o0.1i -F+p1p+gwhite+s << EOF >> $ps
S 0.2i - 0.3i - 1p 	0.5i Cubic spline
S 0.2i - 0.3i - 1p,red	0.5i Akima spline
S 0.2i - 0.3i - 1p,blue	0.5i Linear spline
S 0.2i - 0.3i - 1p,darkgreen 0.5i Nearest neighbor
EOF
# Spline slopes
gmt sample1d t.txt -S0/6 -I0.01 -Fc+1 | gmt psxy -R-0.1/6.1/-3.1/4.5 -J -Bafg -BWsne -By+l"u'(x)" -W1p -O -K -Y3.15i >> $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fl+1 | gmt psxy -R -J -W1p,blue -O -K >> $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fa+1 | gmt psxy -R -J -W1p,red -O -K >> $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fn+1 | gmt psxy -R -J -W1p,darkgreen -O -K >> $ps
gmt pslegend -R -J -O -K -DjTL+w1.9i+o0.1i -F+p1p+gwhite+s << EOF >> $ps
S 0.2i - 0.3i - 1p 	0.5i Cubic spline
S 0.2i - 0.3i - 1p,red	0.5i Akima spline
S 0.2i - 0.3i - 1p,blue	0.5i Linear spline
S 0.2i - 0.3i - 1p,darkgreen 0.5i Nearest neighbor
EOF
# Spline curvatures
gmt sample1d t.txt -S0/6 -I0.01 -Fc+2 | gmt psxy -R-0.1/6.1/-31/31 -J -Bafg  -By+l"u''(x)" -BWsne -W1p -O -K -Y3.15i >> $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fl+2 | gmt psxy -R -J -W1p,blue -O -K >> $ps
gmt sample1d t.txt -S0/6 -I0.01 -Fa+2 | gmt psxy -R -J -W1p,red -O -K >> $ps
gmt pslegend -R -J -O -K -DjTL+w1.9i+o0.1i -F+p1p+gwhite+s << EOF >> $ps
S 0.2i - 0.3i - 1p 	0.5i Cubic spline
S 0.2i - 0.3i - 1p,red	0.5i Akima spline
S 0.2i - 0.3i - 1p,blue	0.5i Linear spline
S 0.2i - 0.3i - 1p,darkgreen 0.5i Nearest neighbor
EOF
gmt psxy -R -J -O -T >> $ps
