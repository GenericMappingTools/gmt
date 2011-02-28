#!/bin/bash
#		GMT EXAMPLE 04
#		$Id: job04.sh,v 1.12 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	3-D mesh plot of Hawaiian topography and geoid
# GMT progs:	grdcontour, grdview, pscoast, pstext
# Unix progs:	echo, rm
#
. ../functions.sh
ps=../example_04.ps
echo '-10  255   0  255' > zero.cpt
echo '  0  100  10  100' >> zero.cpt
grdcontour HI_geoid4.nc -Jm0.45i -E60/30 -R195/210/18/25 -C1 -A5+o -Gd4i -K -P -X1.5i -Y1.5i \
	-U/-1.25i/-1.25i/"Example 4 in Cookbook" > $ps
pscoast -J -E60/30 -R -B2/2NEsw -Gblack -O -K -T209/19.5/1i >> $ps
grdview HI_topo4.nc -J -Jz0.34i -Czero.cpt -E60/30 -R195/210/18/25/-6/4 -N-6/lightgray -Qsm -O -K \
	-B2/2/2:"Topo (km)":neswZ -Y2.2i >> $ps
echo '3.25 5.75 60 0.0 33 BC H@#awaiian@# R@#idge' | pstext -R0/10/0/10 -Jx1i -O >> $ps
rm -f zero.cpt .gmt*
