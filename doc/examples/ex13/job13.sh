#!/bin/sh
#		GMT EXAMPLE 13
#
#		$Id: job13.sh,v 1.4 2003-12-18 02:27:21 pwessel Exp $
#
# Purpose:	Illustrate vectors and contouring
# GMT progs:	grdmath, grdcontour, pstext
# Unix progs:	echo, rm
#
grdmath -R-2/2/-2/2 -I0.1 X Y R2 NEG EXP X MUL = z.grd
grdmath z.grd DDX = dzdx.grd
grdmath z.grd DDY = dzdy.grd
grdcontour dzdx.grd -JX3i -B1/1WSne -C0.1 -A0.5 -K -P -G2i/10 -S4 -T0.1i/0.03i -U"Example 13 in Cookbook" > example_13.ps
grdcontour dzdy.grd -J -B1/1WSne -C0.05 -A0.2 -O -K -G2i/10 -S4 -T0.1i/0.03i -X3.45i >> example_13.ps
grdcontour z.grd -J -B1/1WSne -C0.05 -A0.1 -O -K -G2i/10 -S4 -T0.1i/0.03i -X-3.45i -Y3.45i >> example_13.ps
grdcontour z.grd -J -B1/1WSne -C0.05 -O -K -G2i/10 -S4 -X3.45i >> example_13.ps
grdvector dzdx.grd dzdy.grd -I0.2 -J -O -K -Q0.03i/0.1i/0.09in0.25i -G0 -S5i >> example_13.ps
echo "3.2 3.6 40 0 6 BC z(x,y) = x * exp(-x@+2@+-y@+2@+)" | pstext -R0/6/0/4.5 -Jx1i -O -X-3.45i >> example_13.ps
rm -f z.grd dzdx.grd dzdy.grd .gmt*
