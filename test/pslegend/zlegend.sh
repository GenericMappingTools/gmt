#!/bin/bash
#	$Id$
#
# Testing gmt pslegend capabilities with symbol color lookup

ps=zlegend.ps

gmt makecpt -Ccopper -T0/10/1 > a.cpt
gmt makecpt -Cpolar  -T-5/5/1 > b.cpt
gmt gmtset FONT_ANNOT_PRIMARY 12p

gmt psbasemap -R0/1/0/1 -JX6i/9i -B0 -B+gbisque -P -K -Xc > $ps
gmt pslegend -R -J -DjMC+w4i+jMC+l1.25 -C0.1i/0.1i -F+p+i+gwhite -O >> $ps <<EOF
# Legend test for gmt pslegend
# G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line,
#
H 18p Times-Roman Intensity of Coffee Stains
A a.cpt
D 0.2i 1p
S 0.1i c 0.15i z=0 0.25p 0.3i Symbol color given via z=0 and CPT look-up
S 0.1i c 0.15i z=1 0.25p 0.3i Symbol color given via z=1 and CPT look-up
S 0.1i c 0.15i z=2 0.25p 0.3i Symbol color given via z=2 and CPT look-up
S 0.1i c 0.15i z=3 0.25p 0.3i Symbol color given via z=3 and CPT look-up
S 0.1i c 0.15i z=4 0.25p 0.3i Symbol color given via z=4 and CPT look-up
S 0.1i c 0.15i z=5 0.25p 0.3i Symbol color given via z=5 and CPT look-up
S 0.1i c 0.15i z=6 0.25p 0.3i Symbol color given via z=6 and CPT look-up
S 0.1i c 0.15i z=7 0.25p 0.3i Symbol color given via z=7 and CPT look-up
S 0.1i c 0.15i z=8 0.25p 0.3i Symbol color given via z=8 and CPT look-up
S 0.1i c 0.15i z=9 0.25p 0.3i Symbol color given via z=9 and CPT look-up
S 0.1i c 0.15i z=10 0.25p 0.3i Symbol color given via z=10 and CPT look-up
D 0.2i 1p
G 0.05i
L 9 4 R The CPT file was made via gmt makecpt -Ccopper -T0/10/1
D 0 2p
H 18p Times-Roman Intensity of Political Partisanship
D 0.2i 1p
A b.cpt
S 0.1i s 0.15i z=-5 0.25p 0.3i Symbol color given via z=-5 and CPT look-up
S 0.1i s 0.15i z=-4 0.25p 0.3i Symbol color given via z=-4 and CPT look-up
S 0.1i s 0.15i z=-3 0.25p 0.3i Symbol color given via z=-3 and CPT look-up
S 0.1i s 0.15i z=-2 0.25p 0.3i Symbol color given via z=-2 and CPT look-up
S 0.1i s 0.15i z=-1 0.25p 0.3i Symbol color given via z=-1 and CPT look-up
S 0.1i s 0.15i z=0 0.25p 0.3i Symbol color given via z=0 and CPT look-up
S 0.1i s 0.15i z=1 0.25p 0.3i Symbol color given via z=1 and CPT look-up
S 0.1i s 0.15i z=2 0.25p 0.3i Symbol color given via z=2 and CPT look-up
S 0.1i s 0.15i z=3 0.25p 0.3i Symbol color given via z=3 and CPT look-up
S 0.1i s 0.15i z=4 0.25p 0.3i Symbol color given via z=4 and CPT look-up
S 0.1i s 0.15i z=5 0.25p 0.3i Symbol color given via z=5 and CPT look-up
D 0.2i 1p
G 0.05i
L 9 4 R The CPT file was made via gmt makecpt -Cpolar  -T-5/5/1
EOF
