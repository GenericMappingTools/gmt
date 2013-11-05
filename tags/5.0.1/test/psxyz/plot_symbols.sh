#!/bin/bash
#	$Id$
#
# Plot all the symbols on a 1x1 inch grid pattern

. functions.sh
header "Test psxyz and all the symbols with fill"

psxyz -R0/4/1/6 -Jx1i -P -Bg1 -Gred -W0.25p -p155/35 -S1i -X1i -Y1i -K << EOF > $ps
> Fat pen -W2p
0.5	5.5	0	-
> Plain red symbols -W- -Gred
1.5	5.5	0	bb5
2.5	5.5	0	Bb2
3.5	5.5	0	c
> Switch to green -Ggreen -W1p
0.5	4.5	0	d
1.5	4.5	0	90	1	0.5	e
2.5	4.5	0	g
3.5	4.5	0	h
> Do pattern # 7 -Gp100/7
0.5	3.5	0	i
1.5	3.5	0	90	1	0.5	j
> Do orange -Gorange
2.5	3.5	0	l/M
3.5	3.5	0	n
> Do yellowized pattern # 20 -Gp100/20:Fyellow
0.5	2.5	0	1	0.5	r
1.5	2.5	0	s
2.5	2.5	0	t
> Blue wedges -Gblue
3.5	2.5	0	80	30	w
0.5	1.5	0	30	80	w
> Fat red pen -W2p,red
1.5	1.5	0	x
> Fat pen -W2p
1.5	1.5	0	+
2.5	1.5	0	y
> Dual-colored pattern # 12 -Gp100/12:FredBgreen -W3p,orange
3.5	1.5	0	a
EOF
psxyz -R0/4/1/6/0/3 -Jx1i -Jz1i -O -Bg1/g1/g1 -G0 -W0.25p -p155/35 -S1i -Y4i << EOF >> $ps
> Red cube -Gred
2.5	2.5	3	u
> Blue column -Gblue
2.5	2.5	2	o
EOF

pscmp
