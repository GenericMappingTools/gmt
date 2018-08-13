#!/bin/bash
#		GMT EXAMPLE 18
#
# Purpose:	Illustrates volumes of grids inside contours and spatial
#		selection of data
# GMT modules:	gmtset, gmtselect, gmtspatial, grdclip, grdcontour, grdimage
# GMT modules:	grdmath, grdvolume, makecpt, pscoast, psscale, pstext, psxy
# Unix progs:	rm
#
ps=example_18.ps

# Use spherical gmt projection since SS data define on sphere
gmt set PROJ_ELLIPSOID Sphere FORMAT_FLOAT_OUT %g

# Define location of Pratt seamount and the 400 km diameter
echo "-142.65 56.25 400" > pratt.txt

# First generate gravity image w/ shading, label Pratt, and draw a circle
# of radius = 200 km centered on Pratt.

gmt makecpt -Crainbow -T-60/60 > grav.cpt
gmt grdimage @AK_gulf_grav.nc -I+a45+nt1 -JM5.5i -Cgrav.cpt -B2f1 -P -K -X1.5i \
	-Y5.85i > $ps
gmt pscoast -R@AK_gulf_grav.nc -J -O -K -Di -Ggray -Wthinnest >> $ps
gmt psscale -DJBC+o0/0.4i -R -J -Cgrav.cpt -Bx20f10 -By+l"mGal" -O -K >> $ps
gmt pstext pratt.txt -R -J -O -K -D0.1i/0.1i -F+f12p,Helvetica-Bold+jLB+tPratt >> $ps
gmt psxy pratt.txt -R -J -O -K -SE- -Wthinnest >> $ps

# Then draw 10 mGal contours and overlay 50 mGal contour in green

gmt grdcontour @AK_gulf_grav.nc -J -C20 -B2f1 -BWSEn -O -K -Y-4.85i >> $ps
# Save 50 mGal contours to individual files, then plot them
gmt grdcontour @AK_gulf_grav.nc -C10 -L49/51 -Dsm_%d_%c.txt
gmt psxy -R -J -O -K -Wthin,green sm_*.txt >> $ps
gmt pscoast -R -J -O -K -Di -Ggray -Wthinnest >> $ps
gmt psxy pratt.txt -R -J -O -K -SE- -Wthinnest >> $ps
rm -f sm_*_O.txt	# Only consider the closed contours

# Now determine centers of each enclosed seamount > 50 mGal but only plot
# the ones within 200 km of Pratt seamount.

# First determine mean location of each closed contour and
# add it to the file centers.txt

gmt spatial -Q -fg sm_*_C.txt > centers.txt

# Only plot the ones within 200 km

gmt select -Cpratt.txt+d200k centers.txt -fg | gmt psxy -R -J -O -K -SC0.04i -Gred -Wthinnest >> $ps
gmt psxy -R -J -O -K -ST0.1i -Gyellow -Wthinnest pratt.txt >> $ps

# Then report the volume and area of these seamounts only
# by masking out data outside the 200 km-radius circle
# and then evaluate area/volume for the 50 mGal contour

gmt grdmath -R pratt.txt POINT SDIST = mask.nc
gmt grdclip mask.nc -Sa200/NaN -Sb200/1 -Gmask.nc
gmt grdmath @AK_gulf_grav.nc mask.nc MUL = tmp.nc
area=`gmt grdvolume tmp.nc -C50 -Sk -o1`
volume=`gmt grdvolume tmp.nc -C50 -Sk -o2`
gmt pstext -R -J -O -M -Gwhite -Wthin -Dj0.3i -F+f14p,Helvetica-Bold+jLB -C0.1i >> $ps << END
> -149 52.5 14p 2.6i j
Volumes: $volume mGal\264km@+2@+

Areas: $area km@+2@+
END

# Clean up

rm -f grav.cpt sm_*.txt tmp.nc mask.nc pratt.txt center* gmt.conf
