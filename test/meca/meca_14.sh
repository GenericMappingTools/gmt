#!/bin/bash
#	$Id$
#
#	Test psvelo with either old and new-style vector arguments
#	This currently fails (no PS orig) until we re-implement the V4 vector.
ps=meca_14.ps
cat << EOF > record.txt
0     0     5.0    5.0     2.0    4.0  0.500  3x3
EOF
# Old GMT4 syntax for arrow
gmt psvelo -R-0.5/4/-0.5/3.8 -W1.2p,red -Se0.4i/0.39/12 -BWSne -B1g1 -Jx1i -Ggreen -Eblue -L -N -A0.1i/0.76c/0.3i -P -K -Xc record.txt > $ps
echo GMT4 | gmt pstext -R -J -O -K -F+f18p+cRB -Dj0.1i >> $ps
# New GMT5 syntax for arrow
gmt psvelo -R -J -W1.2p,red -Se0.4i/0.39/12 -BWSne -B1g1 -Ggreen -Eblue -L -N -A1c+p3p+e -O -K -Y4.8i record.txt >> $ps    
echo GMT5 | gmt pstext -R -J -O -F+f18p+cRB -Dj0.1i >> $ps
