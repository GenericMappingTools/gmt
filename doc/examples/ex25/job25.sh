#!/bin/sh
#
#	GMT Example 25  $Id: job25.sh,v 1.3 2004-07-01 17:57:13 pwessel Exp $
#
# Purpose:	Display distribution of antipode types
# GMT progs:	grdlandmask, grdmath, grd2xyz, gmtmath, grdimage, pscoast, pslegend
# Unix progs:	cat
#
# Create D minutes global grid with -1 over oceans and +1 over land
D=30
grdlandmask -Rg -I${D}m -Dc -A500 -N-1/1/1/1/1 -F -Gwetdry.grd
# Manipulate so -1 means ocean/ocean antipode, +1 = land/land, and 0 elsewhere
grdmath wetdry.grd DUP 180 ROTX FLIPUD ADD 2 DIV = key.grd
# Calculate percentage area of each type of antipode match.
grdmath -Rg -I${D}m -F Y COSD 60 $D DIV 360 MUL DUP MUL PI DIV DIV 100 MUL = scale.grd
grdmath key.grd -1 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
ocean=`gmtmath -bi1s -Ca -S key.b SUM UPPER RINT =`
grdmath key.grd 1 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
land=`gmtmath -bi1s -Ca -S key.b SUM UPPER RINT =`
grdmath key.grd 0 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
mixed=`gmtmath -bi1s -Ca -S key.b SUM UPPER RINT =`
# Generate corresponding color table
cat << EOF > key.cpt
-1	blue	0	blue
0	gray	1	gray
1	red	2	red
EOF
# Create the final plot and overlay coastlines
gmtset ANNOT_FONT_SIZE_PRIMARY +10p PLOT_DEGREE_FORMAT dddF
grdimage key.grd -JKs180/9i -B60/30:."Antipodal comparisons":WsNE -K -Ckey.cpt -Y1.2i -U/-0.75i/-0.95i/"Example 25 in Cookbook" > example_25.ps
pscoast -R -J -O -K -Wthinnest -Dc -A500 >> example_25.ps
# Place an explanatory legend below
pslegend -R0/9/0/0.5 -Jx1i/-1i -O -Dx4.5/0/6i/0.3i/TC -Y-0.2i -Fthick << EOF >> example_25.ps
N 3
S 0.15i s 0.2i red  0.25p 0.3i Terrestrial Antipodes [$land %]
S 0.15i s 0.2i blue 0.25p 0.3i Oceanic Antipodes [$ocean %]
S 0.15i s 0.2i gray 0.25p 0.3i Mixed Antipodes [$mixed %]
EOF
rm -f *.grd key.* .gmt*
