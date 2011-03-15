#!/bin/bash
#		GMT EXAMPLE 04c
#		$Id: job04c.sh,v 1.4 2011-03-15 02:06:31 guru Exp $
#
# 3-D perspective color plot of Hawaiian topography and geoid
# GMT progs:	grdcontour, grdgradient, grdimage, grdview, psbasemap, pscoast, pstext
# Unix progs:	echo, rm
#
. ../functions.sh
ps=../example_04c.ps
grdgradient HI_geoid4.nc -A0 -Gg_intens.nc -Nt0.75 -fg
grdgradient HI_topo4.nc -A0 -Gt_intens.nc -Nt0.75 -fg
grdimage HI_geoid4.nc -Ig_intens.nc -R195/210/18/25 -JM6.75i -p60/30 -Cgeoid.cpt -E100 -K \
	-X1.25i -Y1.25i -P -U/-0.5i/-1i/"Example 04c in Cookbook" > $ps
pscoast -R -J -p -B2/2NEsw -Gblack -O -K >> $ps
psscale -R -J -p -D3.375i/4.3i/5i/0.3ih -Cgeoid.cpt -I -O -K -A "-B2:Geoid (m):" >> $ps
psbasemap -R -J -p -O -K -T209/19.5/1i --COLOR_BACKGROUND=red --MAP_TICK_PEN=thinner,red \
	--FONT=red >> $ps
grdview HI_topo4.nc -It_intens.nc -R195/210/18/25/-6/4 -J -JZ3.4i -p -Ctopo.cpt \
	-N-6/lightgray -Qc100 -O -K -B2/2/2:"Topo (km)":neswZ -Y2.2i >> $ps
echo '3.25 5.75 H@#awaiian@# R@#idge' | pstext -R0/10/0/10 -Jx1i \
	-F+f60p,ZapfChancery-MediumItalic+jCB -O >> $ps
rm -f *_intens.nc
