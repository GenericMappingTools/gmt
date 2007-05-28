#!/bin/sh
#	$Id: plot_symbols.sh,v 1.3 2007-05-28 22:21:04 pwessel Exp $
#
# Plot all the symbols on a 1x1 inch grid pattern

echo -n "$0: Test psxy and all the symbols with fill:		"

psxy -R0/4/1/6 -Jx1i -P -B0g1 -M -Gred -W0.25p -S1i -X2i -Y2i << EOF > plot_symbols.ps
> Fat pen -W2p
0.5	5.5	-
> Plain red symbols -W- -Gred
1.5	5.5	bb5
2.5	5.5	Bb2
3.5	5.5	c
> Switch to green -Ggreen -W1p
0.5	4.5	d
1.5	4.5	90	0.5	0.25	e
2.5	4.5	g
3.5	4.5	h
> Do pattern # 7 -Gp100/7
0.5	3.5	i
1.5	3.5	90	1	0.5	j
> Do orange -Gorange
2.5	3.5	l/M
3.5	3.5	n
> Do yellowized pattern # 20 -Gp100/20:Fyellow
0.5	2.5	1	0.5	r
1.5	2.5	s
2.5	2.5	t
> Blue arrow -Gblue
0.5	1.5	30	80	w
3.5	2.5	30	1	vb
> Fat pen -W2p
1.5	1.5	x
2.5	1.5	y
EOF
compare -density 100 -metric PSNR plot_symbols_orig.ps plot_symbols.ps plot_symbols_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAILED]"
else
        echo "[OK"]
        rm -f fail plot_symbols_diff.png log
fi
