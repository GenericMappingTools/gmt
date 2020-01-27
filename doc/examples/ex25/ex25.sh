#!/usr/bin/env bash
#		GMT EXAMPLE 25
#
# Purpose:	Display distribution of antipode types
# GMT modules:	set, grdlandmask, grdmath, grd2xyz, math, grdimage, coast, legend
# Unix progs:	rm
#
# Create D minutes global grid with -1 over oceans and +1 over land
gmt begin ex25
	D=30
	gmt grdlandmask -Rg -I${D}m -Dc -A500 -N-1/1/1/1/1 -r -Gwetdry.nc
	# Manipulate so -1 means ocean/ocean antipode, +1 = land/land, and 0 elsewhere
	gmt grdmath -fg wetdry.nc DUP 180 ROTX FLIPUD ADD 2 DIV = key.nc
	# Calculate percentage area of each type of antipode match.
	gmt grdmath -Rg -I${D}m -r Y COSD 60 $D DIV 360 MUL DUP MUL PI DIV DIV 100 MUL = scale.nc
	gmt grdmath key.nc -1 EQ 0 NAN scale.nc MUL = tmp.nc
	gmt grd2xyz tmp.nc -s -ZTLf > key.b
	ocean=`gmt math -bi1f -Ca -S key.b SUM UPPER RINT =`
	gmt grdmath key.nc 1 EQ 0 NAN scale.nc MUL = tmp.nc
	gmt grd2xyz tmp.nc -s -ZTLf > key.b
	land=`gmt math -bi1f -Ca -S key.b SUM UPPER RINT =`
	gmt grdmath key.nc 0 EQ 0 NAN scale.nc MUL = tmp.nc
	gmt grd2xyz tmp.nc -s -ZTLf > key.b
	mixed=`gmt math -bi1f -Ca -S key.b SUM UPPER RINT =`
	# Generate corresponding color table
	gmt makecpt -Cblue,gray,red -T-1.5/1.5/1 -N
	# Create the final plot and overlay coastlines
	gmt set FONT_ANNOT_PRIMARY +10p FORMAT_GEO_MAP dddF
	gmt grdimage key.nc -JKs180/22c -Bx60 -By30 -BWsNE+t"Antipodal comparisons" -nn
	gmt coast -Wthinnest -Dc -A500
	# Place an explanatory legend below
	gmt legend -DJBC+w15c -Y-0.5c -F+pthick <<- END
	N 3
	S 0.4c s 0.5c red  0.25p 0.75c Terrestrial Antipodes [$land %]
	S 0.4c s 0.5c blue 0.25p 0.75c Oceanic Antipodes [$ocean %]
	S 0.4c s 0.5c gray 0.25p 0.75c Mixed Antipodes [$mixed %]
	END
	rm -f *.nc key.*
gmt end show
