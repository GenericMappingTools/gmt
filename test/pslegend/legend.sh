#!/bin/bash
#	$Id$
#
# Testing pslegend capabilities

. ../functions.sh
header "Test pslegend and its various items"

ps=legend.ps
makecpt -Cpanoply -T-8/8/1 > $$.cpt
gmtset FONT_ANNOT_PRIMARY 12p
pslegend -R0/10/0/10 -JM6i -Dx0.5i/0.5i/5i/3.8i/BL -C0.1i/0.1i -Gazure1 -L1.2 -F+r -B5f1 > $ps <<EOF
# Legend test for pslegend
# G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line,
# H is header, L is label, S is symbol, T is paragraph text, M is map scale, B is colorbar.
#
G -0.1i
H 24 Times-Roman My Map Legend
D 0.2i 1p
N 2
V 0 1p
S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured
S 0.1i e 0.15i yellow 0.25p 0.3i This ellipse is yellow
S 0.1i w 0.15i green 0.25p 0.3i This wedge is green
S 0.1i f 0.25i/-1/0.075ilb blue 0.25p 0.3i This is a fault
S 0.1i - 0.15i - 0.25p,- 0.3i A contour
S 0.1i v 0.25i/0.06i magenta 0.5p 0.3i This is a vector
S 0.1i i 0.15i cyan 0.25p 0.3i This triangle is boring
V 0 1p
N 1
D 0.2i 1p
M 5 5 600+u f
G 0.05i
I SOEST_block4.ras 3i CT
G 0.05i
B $$.cpt 0.2i 0.2i -B0
G 0.05i
L 9 4 R Smith et al., @%5%J. Geophys. Res., 99@%%, 2000
G 0.1i
T Let us just try some simple text that can go on a few lines.
T There is no easy way to predetermine how many lines may be required
T so we may have to adjust the height to get the right size box.
EOF

rm -f $$.cpt

pscmp
