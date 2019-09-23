#!/usr/bin/env bash
#		GMT EXAMPLE 32
#
# Purpose:	Illustrate draping of an image over topography
# GMT modules:	grdcut, grdedit, grdgradient, grdconvert, grdtrack, grdview
# GMT modules:	coast, text, plot3d, makecpt
# Unix progs:	cat, rm
# Credits:	Original by Stephan Eickschen
#
gmt begin ex32
	# Here we get and convert the flag of Europe directly from the web through grdconvert using
	# GDAL support. We take into account the dimension of the flag (1000x667 pixels)
	# for a ratio of 3x2.
	# Because GDAL support will not be standard for most users, we have stored
	# the result, @euflag.nc in this directory.

	Rflag=-R3/9/50/54
	# gmt grdconvert \
	#   http://upload.wikimedia.org/wikipedia/commons/thumb/b/b7/Flag_of_Europe.svg/1000px-Flag_of_Europe.svg.png=gd \
	#   euflag.nc=ns
	# gmt grdedit euflag.nc -fg $Rflag

	# Now get the topography for the same area from GTOPO30 and store it as@topo_32.nc.
	# The DEM file comes from http://eros.usgs.gov/#/Find_Data/Products_and_Data_Available/gtopo30/w020n90
	# We make a gradient grid as well, which we will use to "illuminate" the flag.

	# gmt grdcut W020N90.DEM $Rflag -Gtopo_32.nc=ns

	# The color map assigns "Reflex Blue" to the lower half of the 0-255 range and
	# "Yellow" to the upper half.
	gmt makecpt -C0/51/153,255/204/0 -T0,127,255 -N

	# The next step is the plotting of the image.
	# We use gmt grdview to plot the topography, euflag.nc to give the color, and illum.nc to give
	# the shading.
	Rplot=$Rflag/-10/790
	gmt grdview @topo_32.nc -JM13c $Rplot -C -G@euflag.nc -I+a0/270+ne0.6 -Qc -JZ1c -p157.5/30

	# We now add borders. Because we have a 3-D plot, we want them to be plotted "at elevation".
	# So we write out the borders, pipe them through grdtrack and then plot them with plot3d.
	gmt coast $Rflag -Df -M -N1 | gmt grdtrack -G@topo_32.nc -s+a | gmt plot3d $Rplot -JZ -p -W1p,white

	# Finally, we add dots and names for three cities.
	# Again, gmt grdtrack is used to put the dots "at elevation".
	cat <<- EOF > cities.txt
	05:41:27 50:51:05 Maastricht
	04:21:00 50:51:00 Bruxelles
	07:07:03 50:43:09 Bonn
	EOF

	gmt grdtrack -G@topo_32.nc cities.txt | gmt plot3d $Rplot -JZ -p -Sc7p -W1p,white -Gred
	gmt text -JZ -p -F+f12p,Helvetica-Bold,red+jRM -Dj0.1i/0 cities.txt

	# cleanup
	rm -f cities.txt
gmt end show
