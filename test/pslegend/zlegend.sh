#!/bin/bash
#	$Id$
#
# Testing gmt pslegend capabilities with symbol color lookup

ps=zlegend.ps

gmt makecpt -Crainbow -T0/10/1 > tt.cpt
gmt gmtset FONT_ANNOT_PRIMARY 12p

gmt pslegend -R0/10/0/10 -JM6i -Dx0.5i/0.5i/3i/BL -C0.1i/0.1i -L1.25 -F+p+i -B5f1 -Att.cpt -P > $ps <<EOF
# Legend test for gmt pslegend
# G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line,
#
G -0.1i
H 18p Times-Roman My Symbol Legend
D 0.2i 1p
V 0 1p
S 0.1i c 0.15i z=0 0.25p 0.3i Some label here
S 0.1i c 0.15i z=1 0.25p 0.3i Some label here
S 0.1i c 0.15i z=2 0.25p 0.3i Some label here
S 0.1i c 0.15i z=3 0.25p 0.3i Some label here
S 0.1i c 0.15i z=4 0.25p 0.3i Some label here
S 0.1i c 0.15i z=5 0.25p 0.3i Some label here
S 0.1i c 0.15i z=6 0.25p 0.3i Some label here
S 0.1i c 0.15i z=7 0.25p 0.3i Some label here
S 0.1i c 0.15i z=8 0.25p 0.3i Some label here
S 0.1i c 0.15i z=9 0.25p 0.3i Some label here
S 0.1i c 0.15i z=10 0.25p 0.3i Some label here
V 0 1p
D 0.2i 1p
G 0.05i
L 9 4 R Hopefully that looks nice
EOF
