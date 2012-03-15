#!/bin/bash
#		GMT EXAMPLE 05
#		$Id$
#
# Purpose:	Generate grid and show monochrome 3-D perspective
# GMT progs:	grdgradient, grdmath, grdview, pstext
# Unix progs:	echo, rm
#
ps=example_05.ps
grdmath -R-15/15/-15/15 -I0.3 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = \
	sombrero.nc
echo '-5 128 5 128' > gray.cpt
grdgradient sombrero.nc -A225 -Gintensity.nc -Nt0.75
grdview sombrero.nc -JX6i -JZ2i -B5/5/0.5SEwnZ -N-1/white -Qs -Iintensity.nc -X1.5i -Cgray.cpt \
	-R-15/15/-15/15/-1/1 -K -p120/30 -U/-1.25i/-0.75i/"Example 5 in Cookbook" > $ps
echo "4.1 5.5 z(r) = cos (2@~p@~r/8) @~\327@~e@+-r/10@+" | pstext -R0/11/0/8.5 -Jx1i \
	-F+f50p,ZapfChancery-MediumItalic+jBC -O >> $ps
rm -f gray.cpt sombrero.nc intensity.nc
