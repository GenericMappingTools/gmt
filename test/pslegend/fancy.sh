#!/bin/bash
#	$Id$
#
# Testing pslegend fancy frames

. ../functions.sh
header "Test pslegend frame options"

ps=fancy.ps
makecpt -Cpanoply -T-8/8/1 > $$.cpt
gmtset FONT_ANNOT_PRIMARY 12p
cat << EOF > t.txt
# Legend test for pslegend
# G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line,
# H is header, L is label, S is symbol, T is paragraph text, M is map scale, B is colorbar.
#
G -0.1i
H 24 Times-Roman Map Legend
D 0.2i 1p
V 0 1p
S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured
S 0.1i e 0.15i yellow 0.25p 0.3i This ellipse is yellow
S 0.1i w 0.15i green 0.25p 0.3i This wedge is green
S 0.1i f 0.25i/-1/0.075ilb blue 0.25p 0.3i This is a fault
S 0.1i - 0.15i - 0.25p,- 0.3i A contour
S 0.1i v 0.25i/0.06i magenta 0.25p 0.3i This is a vector
S 0.1i i 0.15i cyan 0.25p 0.3i This triangle is boring
V 0 1p
EOF
psbasemap -R0/6.5/0/10 -Jx1i -P -B0 -K -Y0.5i > $ps
pslegend -R -J -O -K -Dx0.25i/9.75i/2.75i/2.0i/TL -C0.1i/0.1i -Gwhite -L1.2 -F t.txt >> $ps
pslegend -R -J -O -K -Dx6.25i/9.75i/2.75i/2.0i/TR -C0.1i/0.1i -Gwhite -L1.2 -F+r t.txt >> $ps
pslegend -R -J -O -K -Dx0.25i/7.25i/2.75i/2.0i/TL -C0.1i/0.1i -Gwhite -L1.2 -F+p2p+i0.5p t.txt >> $ps
pslegend -R -J -O -K -Dx6.25i/7.25i/2.75i/2.0i/TR -C0.1i/0.1i -Gwhite -L1.2 -F+p2p+i0.5p+s5p/-3p/gray t.txt >> $ps
pslegend -R -J -O -K -Dx0.25i/4.75i/2.75i/2.0i/TL -C0.1i/0.1i -Gwhite -L1.2 -F+p2p,blue+i0.5p,blue t.txt >> $ps
pslegend -R -J -O -K -Dx6.25i/4.75i/2.75i/2.0i/TR -C0.1i/0.1i -Gwhite -L1.2 -F+p2p,blue+i0.5p,blue+s5p/-3p/navy t.txt >> $ps
pslegend -R -J -O -K -Dx0.25i/2.25i/2.75i/2.0i/TL -C0.1i/0.1i -Gwhite -L1.2 -F+i2p,blue+p0.5p,blue t.txt >> $ps
pslegend -R -J -O -K -Dx6.25i/2.25i/2.75i/2.0i/TR -C0.1i/0.1i -Gcornsilk -L1.2 -F+i2p+p0.5p+s-5p/-3p/orange+r t.txt >> $ps
psxy -R -J -O -T >> $ps
rm -f $$.cpt t.txt

pscmp
