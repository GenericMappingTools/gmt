#!/bin/sh
#	$Id: legend.sh,v 1.2 2006-05-22 05:30:47 pwessel Exp $
#
# Testing pslegend capabilities

gmtset ANNOT_FONT_SIZE_PRIMARY 12p
psbasemap -R0/10/0/15 -JM6i -P -B5f1 -K > legend.ps
cat << EOF > t.d
# Legend test for pslegend
# G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line.
# H is header, L is label, S is symbol, > & T is paragraph text
#
G -0.15i
H 24 Times-Roman My Map Legend
G 0.05i
D 0.2i 1p
N 2
V 0 1p
S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured
S 0.1i e 0.15i 255/255/0 0.25p 0.3i This ellipse is yellow
S 0.1i w 0.15i 0/255/0 0.25p 0.3i This wedge is green
S 0.1i f 0.25i/-1/0.075lb 0/0/255 0.25p 0.3i This is a fault
S 0.1i - 0.15i - 0.25tap 0.3i A contour
S 0.1i v 0.25i/0.02/0.06/0.05 255/0/255 0.25p 0.3i This is a vector
S 0.1i i 0.15i 0/255/255 0.25p 0.3i This triangle is boring
V 0 1p
N 1
D 0.2i 1p
I /Users/pwessel/UH/RESOURCES/LOGOS/SOEST_block4.ras 3 CT
G 0.05i
M 5 5 600:km:u f
G 0.1i
L 9 4 R Smith et al., @%5%J. Geophys. Res., 99@%%, 2000
G 0.1i
> - - - - - - - - -
T Let us just try some simple text that can go on a few lines.
T There is no way to predetermine how many lines may be required,
T so we may have to adjust the height to get the right size box.
EOF
#pslegend t.d -R -JM -O -D0.5/0.5/5i/3.3i/LB -C0.1i/0.1i -G240/240/255 -L1.2 -F -S > script.sh
#sh -xv script.sh >> legend.ps
# rm -f script.sh
pslegend t.d -R -JM -O -D0.5/0.5/5i/3.3i/LB -C0.1i/0.1i -G240/240/255 -L1.2 -F >> legend.ps
gv legend.ps &
