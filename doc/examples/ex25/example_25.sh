#!/bin/bash
#		GMT EXAMPLE 25
#		$Id$
#
# Purpose:	Display distribution of antipode types
# GMT progs:	gmtset, grdlandmask, grdmath, grd2xyz, gmtmath, grdimage, pscoast, pslegend
# Unix progs:	cat
#
# Create D minutes global grid with -1 over oceans and +1 over land
ps=example_25.ps
D=30
gmt grdlandmask -Rg -I${D}m -Dc -A500 -N-1/1/1/1/1 -r -Gwetdry.nc
# Manipulate so -1 means ocean/ocean antipode, +1 = land/land, and 0 elsewhere
gmt grdmath -fg wetdry.nc DUP 180 ROTX FLIPUD ADD 2 DIV = key.nc
# Calculate percentage area of each type of antipode match.
gmt grdmath -Rg -I${D}m -r Y COSD 60 $D DIV 360 MUL DUP MUL PI DIV DIV 100 MUL = scale.nc
gmt grdmath -fg key.nc -1 EQ 0 NAN scale.nc MUL = tmp.nc
gmt grd2xyz tmp.nc -s -ZTLf > key.b
ocean=`gmt gmtmath -bi1f -Ca -S key.b SUM UPPER RINT =`
gmt grdmath -fg key.nc 1 EQ 0 NAN scale.nc MUL = tmp.nc
gmt grd2xyz tmp.nc -s -ZTLf > key.b
land=`gmt gmtmath -bi1f -Ca -S key.b SUM UPPER RINT =`
gmt grdmath -fg key.nc 0 EQ 0 NAN scale.nc MUL = tmp.nc
gmt grd2xyz tmp.nc -s -ZTLf > key.b
mixed=`gmt gmtmath -bi1f -Ca -S key.b SUM UPPER RINT =`
# Generate corresponding color table
cat << END > key.cpt
-1.5	blue	-0.5	blue
-0.5	gray	0.5	gray
0.5	red	1.5	red
END
# Create the final plot and overlay coastlines
gmt gmtset FONT_ANNOT_PRIMARY +10p FORMAT_GEO_MAP dddF
gmt grdimage key.nc -JKs180/9i -Bx60 -By30 -BWsNE+t"Antipodal comparisons" -K -Ckey.cpt -Y1.2i -nn > $ps
gmt pscoast -R -J -O -K -Wthinnest -Dc -A500 >> $ps
# Place an explanatory legend below
gmt pslegend -R0/9/0/0.5 -Jx1i -O -Dx4.5i/0+w6i+jTC -Y-0.2i -F+pthick >> $ps << END
N 3
S 0.15i s 0.2i red  0.25p 0.3i Terrestrial Antipodes [$land %]
S 0.15i s 0.2i blue 0.25p 0.3i Oceanic Antipodes [$ocean %]
S 0.15i s 0.2i gray 0.25p 0.3i Mixed Antipodes [$mixed %]
END
rm -f *.nc key.* gmt.conf
