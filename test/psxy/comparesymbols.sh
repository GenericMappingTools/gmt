#!/bin/bash
# Testing that standard symbols plotted in psxy directly are the same
# as plotted when used as macros via a custom symbol.
ps=comparesymbols.ps
gmt psxy -R-1/1/-1/1 -JX4i -P -Bafg1+0.5 -BWSne+t"Using psxy Symbols" -S2i -Wfaint -K -Xc << EOF > $ps
> -Wfaint -Glightgray
0	0	0	30	w
> -G-
0	0	a
0	0	g
0	0	h
0	0	i
> -W4p
0	0	n
> -Wfaint
0	0	x
0	0	s
0	0	d
0	0	t
> -Wfaint,blue
0	0	-
0	0	y
> -Wfaint,red
0	0	0	2i	1i	e
> -Wfaint,orange
0	0	2i	2i r
> -W4p,blue
0	0	90	180	m
> -W1p,green
0	0	c
EOF
echo 	0	0	1i	90	180	m | gmt psxy -R -J -O -S0.1i -W1p,blue,- -K >> $ps
# Now via custom symbols
cat << EOF > junk.def
0	0	1	0	30	w	-Glightgray
0	0	1	a	-Wfaint
0	0	1	g	-Wfaint
0	0	1	h	-Wfaint
0	0	1	i	-Wfaint
0	0	1	n	-W4p
0	0	1	x	-Wfaint
0	0	1	s	-Wfaint
0	0	1	d	-Wfaint
0	0	1	t	-Wfaint
0	0	1	-	-Wfaint,blue
0	0	1	y	-Wfaint,blue
0	0	0	1 0.5 e	-Wfaint,red
0	0	0	1 0.5 e	-Wfaint,red
0	0	1	c	-W1p,green
0	0	1	90 180 m+b+e	-W1p,blue,-
0	0	1	1 r	-Wfaint,orange
# These are custom macros only but shows arc
0.5	0	M	-W2p,red
0	0	1	0	30	A
S
EOF
echo 0 0 | gmt psxy -R -J -O -Bafg1+0.5 -Skjunk/2i -Y4.75i -BWsne+t"Using Custom Symbols" >> $ps
