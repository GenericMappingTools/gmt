#!/bin/bash
#	$Id$
#
# Testing gmt pslegend fancy frames

ps=fancy.ps

gmt makecpt -Cpanoply -T-8/8/1 > tt.cpt
gmt gmtset FONT_ANNOT_PRIMARY 12p
cat << EOF > tt.txt
# Legend test for gmt pslegend
# G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line,
# H is ps=fancy.ps
#
G -0.1i
H 24 Times-Roman Map Legend
D 0.2i 1p
V 0 1p
S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured
S 0.1i e 0.15i yellow 0.25p 0.3i This ellipse is yellow
S 0.1i w 0.15i green 0.25p 0.3i This wedge is green
S 0.1i f 0.25i blue 0.25p 0.3i This is a fault
S 0.1i - 0.15i - 0.25p,- 0.3i A contour
S 0.1i v 0.25i magenta 0.5p 0.3i This is a vector
S 0.1i i 0.15i cyan 0.25p 0.3i This triangle is boring
V 0 1p
EOF
gmt psbasemap -R0/6.5/0/10 -Jx1i -P -B0 -K -Y0.5i > $ps
gmt pslegend -R -J -O -K -Dx0.25i/9.75i/2.75i/TL -C0.1i/0.1i -L1.2 -F+p+gwhite tt.txt >> $ps
gmt pslegend -R -J -O -K -Dx6.25i/9.75i/2.75i/TR -C0.1i/0.1i -L1.2 -F+p+gwhite+r tt.txt >> $ps
gmt pslegend -R -J -O -K -Dx0.25i/7.25i/2.75i/TL -C0.1i/0.1i -L1.2 -F+p2p+gwhite+i0.5p tt.txt >> $ps
gmt pslegend -R -J -O -K -Dx6.25i/7.25i/2.75i/TR -C0.1i/0.1i -L1.2 -F+p2p+gwhite+i0.5p+s5p/-3p/gray tt.txt >> $ps
gmt pslegend -R -J -O -K -Dx0.25i/4.75i/2.75i/TL -C0.1i/0.1i -L1.2 -F+p2p,blue+gwhite+i0.5p,blue tt.txt >> $ps
gmt pslegend -R -J -O -K -Dx6.25i/4.75i/2.75i/TR -C0.1i/0.1i -L1.2 -F+p2p,blue+gwhite+i0.5p,blue+s5p/-3p/navy tt.txt >> $ps
gmt pslegend -R -J -O -K -Dx0.25i/2.25i/2.75i/TL -C0.1i/0.1i -L1.2 -F+i2p,blue+gwhite+p0.5p,blue tt.txt >> $ps
gmt pslegend -R -J -O -K -Dx6.25i/2.25i/2.75i/TR -C0.1i/0.1i -L1.2 -F+gcornsilk+i2p+p0.5p+s-5p/-3p/orange+r tt.txt >> $ps
gmt psxy -R -J -O -T >> $ps

