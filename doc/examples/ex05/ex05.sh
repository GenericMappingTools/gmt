#!/usr/bin/env bash
#		GMT EXAMPLE 05
#
# Purpose:	Generate grid and show monochrome 3-D perspective
# GMT modules:	grdmath, grdview, text, makecpt
# Unix progs:	echo, rm
#
gmt begin ex05
	gmt grdmath -R-15/15/-15/15 -I0.3 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = sombrero.nc
	gmt makecpt -C128 -T-5,5 -N
	gmt grdview sombrero.nc -JX5i -JZ1.75i -B -Bz0.5 -BSEwnZ -N-1+gwhite -Qs -I+a225+nt0.75 \
		-C -R-15/15/-15/15/-1/1 -p120/30
	echo "3.5 4.5 z(r) = cos (2@~p@~r/8) @~\327@~e@+-r/10@+" | gmt text -R0/10/0/10 -Jx1i \
		-F+f50p,ZapfChancery-MediumItalic+jBC
	rm -f sombrero.nc
gmt end show
