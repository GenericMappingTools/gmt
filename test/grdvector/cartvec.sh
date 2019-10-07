#!/usr/bin/env bash
# Switch sign for scaling and try with/without -T
# Red is original, blue subject to -T
ps=cartvec.ps
gmt xyz2grd -R-1/1/-2/2 -I1 -Gr.nc << EOF
0	0	1
0	-1	0.5
EOF
gmt xyz2grd -R-1/1/-2/2 -I1 -Gaz.nc << EOF
0	0	60
0	-1	-100
EOF

gmt grdvector r.nc az.nc -A -JX2.5i -Q0.3i+e -W3p,red -Gred -Si1i -P -K -Bafg10 -BWSne > $ps
gmt grdvector r.nc az.nc -A -T -J -Q0.1i+ec -W1p,blue -Gblue -Si1i -O -K >> $ps
gmt pstext -R -J -O -K -F+f14p+jTL+cTL+tORIG -Dj0.1i >> $ps
gmt grdvector r.nc az.nc -A -JX2.5i/-2.5i -Q0.3i+e -W3p,red -Gred -Si1i -O -K -Bafg10 -BWSne -X3.75i >> $ps
gmt grdvector r.nc az.nc -A -T -J -Q0.1i+ec -W1p,blue -Gblue -Si1i -O -K >> $ps
gmt pstext -R -J -O -K -F+f14p+jTL+cTL+t"NEG Y" -Dj0.1i >> $ps
gmt grdvector r.nc az.nc -A -JX-2.5i/2.5i -Q0.3i+e -W3p,red -Gred -Si1i -O -K -Bafg10 -BWSne -X-3.75i -Y4i >> $ps
gmt grdvector r.nc az.nc -A -T -J -Q0.1i+ec -W1p,blue -Gblue -Si1i -O -K >> $ps
gmt pstext -R -J -O -K -F+f14p+jTL+cTL+t"NEG X" -Dj0.1i >> $ps
gmt grdvector r.nc az.nc -A -JX-2.5i/-2.5i -Q0.3i+e -W3p,red -Gred -Si1i -O -K -Bafg10 -BWSne -X3.75i >> $ps
gmt grdvector r.nc az.nc -A -T -J -Q0.1i+ec -W1p,blue -Gblue -Si1i -O -K >> $ps
gmt pstext -R -J -O -F+f14p+jTL+cTL+t"NEG X,Y" -Dj0.1i >> $ps
#rm -f az.nc r.nc t.cpt
