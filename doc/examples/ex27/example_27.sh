#!/bin/bash
#		GMT EXAMPLE 27
#		$Id$
#
# Purpose:	Illustrates how to plot Mercator img grids
# GMT progs:	makecpt, mapproject, grdgradient, grdimage, grdinfo, pscoast
# GMT supplement: img2grd (to read Sandwell/Smith img files)
# Unix progs:	rm, grep, $AWK
#
ps=example_27.ps

# Gravity in tasman_grav.nc is in 0.1 mGal increments and the grid
# is already in projected Mercator x/y units.
# First get gradients.

gmt grdgradient tasman_grav.nc -Nt1 -A45 -Gtasman_grav_i.nc

# Make a suitable cpt file for mGal

gmt makecpt -T-120/120/240 -Z -Crainbow > grav.cpt

# Since this is a Mercator grid we use a linear projection

gmt grdimage tasman_grav.nc=ns/0.1 -Itasman_grav_i.nc -Jx0.25i -Cgrav.cpt -P -K > $ps

# Then use gmt pscoast to plot land; get original -R from grid remark
# and use Mercator gmt projection with same scale as above on a spherical Earth

R=`gmt grdinfo tasman_grav.nc | grep Remark | $AWK '{print $NF}'`

gmt pscoast $R -Jm0.25i -Ba10f5 -BWSne -O -K -Gblack --PROJ_ELLIPSOID=Sphere \
	-Cwhite -Dh+ --FORMAT_GEO_MAP=dddF >> $ps

# Put a color legend in top-left corner of the land mask

gmt psscale -DjTL+o1c+w2i/0.15i -R -J -Cgrav.cpt -Bx50f10 -By+lmGal -I -O -F+gwhite+p1p >> $ps

# Clean up

rm -f grav.cpt *_i.nc
