#!/bin/bash
#	$Id$
# Deal with gmtselect action on lines
ps=selectlines.ps
cat << EOF > lines.txt
> This line is inside
1	1
2	2
1	3
> This line is entirely outside
-5	-2
-4	-1
-3	0
> This line crosses boundary
-2	-2
1	0.5
2	1
> This also crosses boundary
5	3
3	2
2	3
EOF
cat << EOF > pol.txt
0	0
4	0
4	4
0	4
EOF

# Get what gmtselect lets through
gmt select -R0/4/0/4 lines.txt > tmp.txt
# Lay down lines and nodes
gmt psxy -R-5/5/-3/5 -Jx0.5i -P -Xc -Baf -W2p -K lines.txt > $ps
gmt psxy -R -J -O -K -Sc0.2i lines.txt -N >> $ps
gmt psxy -R -J -O -K -W2p,green tmp.txt >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Ggreen tmp.txt >> $ps
gmt psxy -R -J -O -K -L -W0.25p,- pol.txt >> $ps
echo -5 5 "gmtselect -R" | gmt pstext -R -J -O -K -F+f12p+jLT -Dj0.1i >> $ps
# Same with gmtspatial
gmt psxy -R -J -O -Y4.75i -Baf -W2p -K lines.txt >> $ps
gmt psxy -R -J -O -K -Sc0.2i lines.txt -N >> $ps
gmt spatial -R -Fl -Tpol.txt lines.txt > tmp.txt
gmt psxy -R -J -O -K -W2p,green tmp.txt >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Ggreen tmp.txt >> $ps
gmt psxy -R -J -O -K -L -W0.25p,- pol.txt >> $ps
echo -5 5 "gmtspatial -Fl -T" | gmt pstext -R -J -O -F+f12p+jLT -Dj0.1i >> $ps
