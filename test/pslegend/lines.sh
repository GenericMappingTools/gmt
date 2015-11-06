#!/bin/bash
#	$Id$
#
# Testing gmt pslegend capabilities for fronts and quoted lines

ps=lines.ps

gmt gmtset FONT_ANNOT_PRIMARY 12p

gmt pslegend -R0/10/0/7 -JM6i -Dx0.5i/0.5i+w5i+jBL+l1.75 -F+p1p -C0.1i/0.1i -B5f1 -P -Xc > $ps <<EOF
# Legend test for gmt pslegend
S 1.3i f+r+f	2i/0.6i/0.25i	-	1.0p		2.6i	A simple fault symbol
S 1.3i f+r+s+o0.4i+p1p	2i/0.6i/0.25i	-	2.0p		2.6i	Right lateral strike-slip
S 1.3i f+l+t	2i/0.2i/0.075i	red	1.0p		2.6i	Subduction zone
S 1.3i f+r+c	2i/0.2i/0.075i	red	1.0p		2.6i	Warm front to the south
S 1.3i f+l+b+p	2i/0.2i/0.075i	blue	1.0p,green		2.6i	Demarcation line
S 1.3i f+c	2i/0.4i/0.1i	white	1.0p		2.6i	Some path goes here
S 1.3i f+c+o0.2i+p0.25p,yellow	2i/0.4i/0.1i	gray	1.0p,.		2.6i	Another path is offset
S 1.3i qn1:+lPerimeter	2i 		-	2.5p,orange	2.6i	A boundary between things
S 1.3i qn2:+l5m	2i		-	1.5p		2.6i	5-m contour line
S 1.3i qn8:+l\267	2i	-	1.5p,red		2.6i	Frontier between angry folk
S 1.3i qN2:+lA+x	2i	-	1.5p,brown		2.6i	A geologic crossection
EOF
