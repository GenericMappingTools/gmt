#!/usr/bin/env bash
#
# Test psxy -Zfile[+t|T] for coloring and/or transparency of lines
# With +t we expect no z-value (so pen/fill is the same) but transparency.
# With +T we expect both transparency and z-value as two last columns in
# file given by -Z.

ps=variable_lines3D.ps
cat << EOF > z.txt
# transparency z-value
35	2
85	8
EOF
cat << EOF > tr.txt
# transparency only
35
85
EOF
cat << EOF > t.txt
# File with two lines
> first header
0	1	0
1	2	1
> yet another
3	2	1
4	3	0
EOF
gmt begin variable_lines ps
	gmt makecpt -T0/10 -Cjet
	gmt subplot begin 2x2 -Fs8c -R-1/5/-1/5/0/2
	# Constant red lines
	gmt plot3d -W15p,red t.txt -c -JZ5c -p195/30
	# Colored lines, no transparency
	gmt plot3d -W15p+z t.txt -C -Zz.txt -c -p195/30
	# Transparent lines, no color
	gmt plot3d -W15p+z t.txt -Ztr.txt+t -c -p195/30
	# Transparent and colored lines
	gmt plot3d -W15p+z -Zz.txt+T t.txt -C -c -p195/30
	gmt subplot end
gmt end show
