#!/bin/bash
#		GMT EXAMPLE 29
#		$Id: job29.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Illustrates spherical surface gridding with Green's function of splines
# GMT progs:	makecpt, grdcontour, grdgradient, grdimage, grdmath greenspline, psscale, pstext
# Unix progs:	rm, echo
#
. ../functions.sh
ps=../example_29.ps

# This example uses 370 radio occultation data for Mars to grid the topography.
# Data and information from Smith, D. E., and M. T. Zuber (1996), The shape of
# Mars and the topographic signature of the hemispheric dichotomy, Science, 271, 184–187.

# Make Mars ellipsoid given their three best-fitting axes:
a=3399.472
b=3394.329
c=3376.502
grdmath -Rg -I4 -F X COSD $a DIV DUP MUL X SIND $b DIV DUP MUL ADD Y COSD DUP MUL MUL Y SIND $c DIV \
	DUP MUL ADD SQRT INV = ellipsoid.nc

#  Do both Parker and Wessel/Becker solutions (tension = 0.9975)
greenspline -Rellipsoid.nc mars370.in -D4 -Sp -Gmars.nc
greenspline -Rellipsoid.nc mars370.in -D4 -SQ0.9975/5001 -Gmars2.nc
# Scale to km and remove ellipsoid
grdmath mars.nc 1000 DIV ellipsoid.nc SUB = mars.nc
grdmath mars2.nc 1000 DIV ellipsoid.nc SUB = mars2.nc
makecpt -Crainbow -T-7/15/1 -Z > mars.cpt
grdgradient mars2.nc -M -Ne0.75 -A45 -Gmars2_i.nc
grdimage mars2.nc -Imars2_i.nc -Cmars.cpt -B30g30Wsne -JH0/6i -P -K -Ei \
	-U"Example 29 in Cookbook" --ANNOT_FONT_SIZE_PRIMARY=12 > $ps
grdcontour mars2.nc -J -O -K -C1 -A5 -Glz+/z- >> $ps
psxy -Rg -J -O -K -Sc0.045i -Gblack mars370.in  >> $ps
echo "0 90 14 0 1 LB b)" | pstext -R -J -O -K -N -D-3i/-0.2i >> $ps
grdgradient mars.nc -M -Ne0.75 -A45 -Gmars_i.nc
grdimage mars.nc -Imars_i.nc -Cmars.cpt -B30g30Wsne -J -O -K -Y3.6i -Ei --ANNOT_FONT_SIZE_PRIMARY=12 >> $ps
grdcontour mars.nc -J -O -K -C1 -A5 -Glz+/z- >> $ps
psxy -Rg -J -O -K -Sc0.045i -Gblack mars370.in  >> $ps
psscale -Cmars.cpt -O -K -D3i/-0.1i/5i/0.1ih -I --ANNOT_FONT_SIZE_PRIMARY=12 -B2f1/:km: >> $ps
echo "0 90 14 0 1 LB a)" | pstext -R -J -O -N -D-3i/-0.2i >> $ps
# Clean up
rm -f *.nc mars.cpt .gmt*
