#!/bin/bash
#		GMT EXAMPLE 27
#		$Id$
#
# Purpose:	Illustrates how to plot Mercator img grids
# GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast
# GMT supplement: img2grd (to read Sandwell/Smith img files)
# Unix progs:	rm, grep, $AWK
#
ps=example_27.ps

# Gravity in tasman_grav.nc is in 0.1 mGal increments and the grid
# is already in projected Mercator x/y units.
# First get gradients.

grdgradient tasman_grav.nc -Nt1 -A45 -Gtasman_grav_i.nc

# Make a suitable cpt file for mGal

makecpt -T-120/120/240 -Z -Crainbow > grav.cpt

# Since this is a Mercator grid we use a linear projection

grdimage tasman_grav.nc=ns/0.1 -Itasman_grav_i.nc -Jx0.25i -Cgrav.cpt -P -K \
	-U"Example 27 in Cookbook" > $ps

# Then use pscoast to plot land; get original -R from grid remark
# and use Mercator projection with same scale as above on a spherical Earth

R=`grdinfo tasman_grav.nc | grep Remark | $AWK '{print $NF}'`

pscoast $R -Jm0.25i -Ba10f5WSne -O -K -Gblack --PROJ_ELLIPSOID=Sphere \
	-Cwhite -Dh+ --FORMAT_GEO_MAP=dddF >> $ps

# Put a color legend on top of the land mask justified with 147E,31S

#echo 147E 31S 1i 2.5i | psxy -R -J -O -K -Sr -D0.25i/0.05i -Gwhite -W1p \
#	--PROJ_ELLIPSOID=Sphere >> $ps
pos=`echo 147E 31S | mapproject -R -J --PROJ_ELLIPSOID=Sphere | $AWK '{printf "%si/%si\n", $1, $2}'`
psscale -D$pos/2i/0.15i -Cgrav.cpt -B50f10/:mGal: -I -O -T+gwhite+p1p >> $ps

# Clean up

rm -f grav.cpt *_i.nc
