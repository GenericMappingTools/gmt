#!/usr/bin/env bash
#		GMT EXAMPLE 29
#
# Purpose:	Illustrates spherical surface gridding with Green's function of splines
# GMT modules:	makecpt, grdcontour, grdimage, grdmath greenspline, colorbar, text
# Unix progs:	echo
#
gmt begin ex29 ps

	# This example uses 370 radio occultation data for Mars to grid the topography.
	# Data and information from Smith, D. E., and M. T. Zuber (1996), The shape of
	# Mars and the topographic signature of the hemispheric dichotomy, Science, 271, 184-187.
	
	# Make Mars PROJ_ELLIPSOID given their three best-fitting axes:
	a=3399.472
	b=3394.329
	c=3376.502
	gmt grdmath -Rg -I4 -r X COSD $a DIV DUP MUL X SIND $b DIV DUP MUL ADD Y COSD DUP MUL MUL Y \
		SIND $c DIV DUP MUL ADD SQRT INV = PROJ_ELLIPSOID.nc
	
	#  Do both Parker and Wessel/Becker solutions (tension = 0.9975)
	gmt greenspline -RPROJ_ELLIPSOID.nc @mars370.txt -D4 -Sp -Gmars.nc
	gmt greenspline -RPROJ_ELLIPSOID.nc @mars370.txt -D4 -Sq0.9975 -Gmars2.nc
	# Scale to km and remove PROJ_ELLIPSOID
	gmt grdmath mars.nc 1000 DIV PROJ_ELLIPSOID.nc SUB = mars.nc
	gmt grdmath mars2.nc 1000 DIV PROJ_ELLIPSOID.nc SUB = mars2.nc
	gmt makecpt -Crainbow -T-7/15
	gmt grdimage mars2.nc -I+ne0.75+a45 -B30g30 -BWsne -Rg -JH0/7i -E200 --FONT_ANNOT_PRIMARY=12p -X0.75i
	gmt grdcontour mars2.nc -C1 -A5 -Glz+/z-
	gmt plot -Sc0.045i -Gblack @mars370.txt
	echo "0 90 b)" | gmt text -N -D-3.5i/-0.2i -F+f14p,Helvetica-Bold+jLB
	gmt grdimage mars.nc -I+ne0.75+a45 -B30g30 -BWsne -Y4.2i -E200 --FONT_ANNOT_PRIMARY=12p
	gmt grdcontour mars.nc -C1 -A5 -Glz+/z-
	gmt plot -Sc0.045i -Gblack @mars370.txt
	gmt colorbar -DJBC+o0/0.15i+w6i/0.1i -I --FONT_ANNOT_PRIMARY=12p -Bx2f1 -By+lkm
	echo "0 90 a)" | gmt text -N -D-3.5i/-0.2i -F+f14p,Helvetica-Bold+jLB
gmt end
