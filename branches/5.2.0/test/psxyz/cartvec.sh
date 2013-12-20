#!/bin/bash
#	$Id$
ps=cartvec.ps

cat << EOF > vec.txt
0	0	0	60	1.25i
0	0	0	-100	0.75i
EOF
gmt psxyz vec.txt -R-1/1/-1/1 -JX2.5i -p165/35 -P -Bafg10 -BWSne -Sv0.25i+e -Gred -W3p -K > $ps
gmt psxyz vec.txt -R -O -K -Bafg10 -p -BWSne -Sv0.25i+e -Gred -W3p -X3.5i -JX2.5i/-2.5i >> $ps
gmt psxyz vec.txt -R -O -K -Bafg10 -p -BWSne -Sv0.25i+e -Gred -W3p -X-3.5i -Y4i -JX-2.5i/2.5i >> $ps
gmt psxyz vec.txt -R -O -Bafg10 -p -BWSne -Sv0.25i+e -Gred -W3p -X3.5i -JX-2.5i/-2.5i >> $ps
