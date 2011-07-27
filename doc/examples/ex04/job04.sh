#!/bin/bash
#		GMT EXAMPLE 04
#		$Id$
#
# Purpose:	3-D mesh plot of Hawaiian topography and geoid
# GMT progs:	grdcontour, grdview, pscoast, pstext
# Unix progs:	echo, rm
#
. ../functions.sh
ps=../example_04.ps
echo '-10  255   0  255' > zero.cpt
echo '  0  100  10  100' >> zero.cpt
grdcontour HI_geoid4.nc -R195/210/18/25 -Jm0.45i -p60/30 -C1 -A5+o -Gd4i -K -P \
	-X1.25i -Y1.25i -U/-0.5i/-1i/"Example 4 in Cookbook" > $ps
pscoast -R -J -p -B2/2NEsw -Gblack -O -K -T209/19.5/1i >> $ps
grdview HI_topo4.nc -R195/210/18/25/-6/4 -J -Jz0.34i -p -Czero.cpt -O -K \
	-N-6/lightgray -Qsm -B2/2/2:"Topo (km)":neswZ -Y2.2i >> $ps
echo '3.25 5.75 H@#awaiian@# R@#idge' | pstext -R0/10/0/10 -Jx1i \
	-F+f60p,ZapfChancery-MediumItalic+jCB -O >> $ps
rm -f zero.cpt
