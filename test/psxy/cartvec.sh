#!/bin/bash
#	$Id$
ps=cartvec.ps

cat << EOF > vec.txt
0	0	60	1.5i
0	0	-100	1i
EOF
gmt psxy vec.txt -R-1/1/-1/1 -JX3i -P -Bafg10 -BWSne -Sv0.25i+e -Gred -W3p -K > $ps
gmt psxy vec.txt -R -O -K -Bafg10 -BWSne -Sv0.25i+e -Gred -W3p -X3.75i -JX3i/-3i >> $ps
gmt psxy vec.txt -R -O -K -Bafg10 -BWSne -Sv0.25i+e -Gred -W3p -X-3.75i -Y4i -JX-3i/3i >> $ps
gmt psxy vec.txt -R -O -Bafg10 -BWSne -Sv0.25i+e -Gred -W3p -X3.75i -JX-3i/-3i >> $ps
