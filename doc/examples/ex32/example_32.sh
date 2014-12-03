#!/bin/bash
#		GMT EXAMPLE 32
#		$Id$
#
# Purpose:	Illustrate draping of an image over topography
# GMT progs:	grdcut, grdedit, grdgradient, grdconvert, grdtrack, grdview
# GMT progs:	pscoast, pstext, psxyz
# Unix progs:	cat, rm
# Credits:	Original by Stephan Eickschen
#
ps=example_32.ps

# Here we get and convert the flag of Europe directly from the web through grdconvert using
# GDAL support. We take into account the dimension of the flag (1000x667 pixels)
# for a ratio of 3x2.
# Because GDAL support will not be standard for most users, we have stored
# the result, euflag.nc in this directory.

Rflag=-R3/9/50/54
# gmt grdconvert \
#   http://upload.wikimedia.org/wikipedia/commons/thumb/b/b7/Flag_of_Europe.svg/1000px-Flag_of_Europe.svg.png=gd \
#   euflag.nc=ns
# gmt grdedit euflag.nc -fg $Rflag

# Now get the topography for the same area from GTOPO30 and store it as topo.nc.
# The DEM file comes from http://eros.usgs.gov/#/Find_Data/Products_and_Data_Available/gtopo30/w020n90
# We make an gradient grid as well, which we will use to "illuminate" the flag.

# gmt grdcut W020N90.DEM $Rflag -Gtopo.nc=ns
gmt grdgradient topo.nc -A0/270 -Gillum.nc -Ne0.6

# The color map assigns "Reflex Blue" to the lower half of the 0-255 range and
# "Yellow" to the upper half.
cat << EOF > euflag.cpt
0	0/51/153	127	0/51/153
127	255/204/0	255	255/204/0
EOF

# The next step is the plotting of the image.
# We use gmt grdview to plot the topography, euflag.nc to give the color, and illum.nc to give
# the shading.

Rplot=$Rflag/-10/790
gmt grdview topo.nc -JM13c $Rplot -Ceuflag.cpt -Geuflag.nc -Iillum.nc -Qc -JZ1c -p157.5/30 -P -K > $ps

# We now add borders. Because we have a 3-D plot, we want them to be plotted "at elevation".
# So we write out the borders, pipe them through grdtack and then plot them with psxyz.

gmt pscoast $Rflag -Df -M -N1 | gmt grdtrack -Gtopo.nc -sa | gmt psxyz $Rplot -J -JZ -p -W1p,white \
	-O -K >> $ps

# Finally, we add dots and names for three cities.
# Again, gmt grdtrack is used to put the dots "at elevation".

cat << EOF > cities.txt
05:41:27 50:51:05 Maastricht
04:21:00 50:51:00 Bruxelles
07:07:03 50:43:09 Bonn
EOF

gmt grdtrack -Gtopo.nc -sa cities.txt | gmt psxyz -i0,1,3 $Rplot -J -JZ -p -Sc7p -W1p,white -Gred \
	-K -O >> $ps
gmt pstext $Rplot -J -JZ -p -F+f12p,Helvetica-Bold,red+jRM -Dj0.1i/0.0i -O cities.txt >> $ps

# cleanup

rm -f gmt.conf euflag.cpt illum.nc cities.txt
