#!/usr/bin/env bash
#
#	Test psvelo with either old and new-style vector arguments
#	The V4 vector is selected when -A is in old GMT4 format.
ps=geodesy_04.ps
cat << EOF > record.txt
0     0     5.0    5.0     2.0    4.0  0.500  3x3
EOF
# Old GMT4 syntax for arrow
gmt psvelo -R-0.5/4/-0.5/3.8 -W1.2p,red -Se0.4i/0.39+f12p -BWSne -B1g1 -Jx1i -Ggreen -Eblue -L -N -A0.1i/0.76c/0.3i -P -K -Xc record.txt > $ps
gmt pstext -R -J -O -K -F+f18p+cRB+tGMT4 -Dj0.1i >> $ps
# New GMT5 syntax for arrow
gmt psvelo -R -J -W1.2p,red -Se0.4i/0.39+f12p -BWSne -B1g1 -Ggreen -Eblue -L -N -A0.3i+p3p+e+a90 -O -K -Y4.8i record.txt >> $ps   
gmt pstext -R -J -O -F+f18p+cRB+tGMT5 -Dj0.1i >> $ps
