#!/bin/bash
#		GMT EXAMPLE 27
#		$Id: job27.sh,v 1.7 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Illustrates how to plot Mercator img grids
# GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast
# GMT supplement: img2grd (to read Sandwell/Smith img files)
# Unix progs:	rm, grep, $AWK
#
. ../functions.sh
ps=../example_27.ps

# First extract a chunk of faa and retain short int precision to
# save disk space.  Gravity is thus in 0.1 mGal increments.
# Next get gradients.  The grid's region is in Mercator x/y units

#img2grd grav.15.2.img -R145/170/-50/-25 -M -C -T1 -Gtasman_grav.nc=ns
grdgradient tasman_grav.nc -Nt1 -A45 -Gtasman_grav_i.nc

# Make a suitable cpt file for mGal

makecpt -T-120/120/10 -Z -Crainbow > grav.cpt

# Since this is a Mercator grid we use a linear projection

grdimage tasman_grav.nc=ns/0.1 -Itasman_grav_i.nc -Jx0.25i -Cgrav.cpt -P -K \
	-U"Example 27 in Cookbook" > $ps

# Then use pscoast to plot land; get original -R from grid remark
# and use Mercator projection with same scale as above on a spherical Earth

R=`grdinfo tasman_grav.nc | grep Remark | $AWK '{print $NF}'`

pscoast $R -Jm0.25i -Ba10f5WSne -O -K -Gblack --ELLIPSOID=Sphere \
	-Cwhite -Dh+ --PLOT_DEGREE_FORMAT=dddF >> $ps

# Put a color legend on top of the land mask justified with 147E,31S

echo 147E 31S | mapproject -R -J --ELLIPSOID=Sphere > tmp
echo 147E 31S 1 2.5 | psxy -R -J -O -K -Sr -D0.25i/0.05i -Gwhite -W1p --ELLIPSOID=Sphere --MEASURE_UNIT=inch >> $ps
pos=`$AWK '{printf "%si/%si\n", $1, $2}' tmp`
psscale -D$pos/2i/0.15i -Cgrav.cpt -B50f10/:mGal: -I -O >> $ps

# Clean up

rm -f grav.cpt *_i.nc .gmt* tmp
