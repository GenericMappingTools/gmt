#!/bin/sh
#		GMT EXAMPLE 04
#
#		$Id: job04.sh,v 1.7 2004-05-26 03:25:22 pwessel Exp $
#
# Purpose:	3-D mesh plot of Hawaiian topography and geoid
# GMT progs:	grdcontour, grdview, pscoast, pstext, psxyz
# Unix progs:	echo, rm
#
echo '-10     255     0       255' > zero.cpt
echo '0       100     10      100' >> zero.cpt
grdcontour HI_geoid4.grd -Jm0.45i -E60/30 -R195/210/18/25 -C1 -A5 -Gd4i -K -P -X1.5i -Y1.5i -U/-1.25i/-1.25i/"Example 4 in Cookbook" > example_04.ps
pscoast -J -E60/30 -R -B2/2NEsw -Gblack -O -K -T209/19.5/1i >> example_04.ps
grdview HI_topo4.grd -J -Jz0.34i -Czero.cpt -E60/30 -R195/210/18/25/-6/4 -N-6/lightgray -Qsm -O -K -B2/2/2:"Topo (km)":neswZ -Y2.2i >> example_04.ps
echo '3.25 5.75 60 0.0 33 BC H@#awaiian@# R@#idge' | pstext -R0/10/0/10 -Jx1i -O >> example_04.ps
rm -f zero.cpt .gmt*
sh job4c.sh
