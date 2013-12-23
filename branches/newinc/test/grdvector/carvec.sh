#!/bin/bash
# $Id: vectors.sh 12115 2013-09-03 23:22:48Z fwobbe $

ps=carvec.ps
gmt xyz2grd -R-1/1/-2/2 -I1 -Gr.nc << EOF
0	0	1
0	-1	0.5
EOF
gmt xyz2grd -R-1/1/-2/2 -I1 -Gaz.nc << EOF
0	0	60
0	-1	-100
EOF

gmt grdvector r.nc az.nc -A -JX2.5i -Q0.2i+e -W2p -Gred -Si1i -P -K -Bafg10 -BWSne > $ps
gmt grdvector r.nc az.nc -A -JX2.5i/-2.5i -Q0.2i+e -W2p -Gred -Si1i -O -K -Bafg10 -BWSne -X3.75i >> $ps
gmt grdvector r.nc az.nc -A -JX-2.5i/2.5i -Q0.2i+e -W2p -Gred -Si1i -O -K -Bafg10 -BWSne -X-3.75i -Y4i >> $ps
gmt grdvector r.nc az.nc -A -JX-2.5i/-2.5i -Q0.2i+e -W2p -Gred -Si1i -O -Bafg10 -BWSne -X3.75i >> $ps
#rm -f az.nc r.nc t.cpt
