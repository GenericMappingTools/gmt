#!/bin/bash
#	$Id$
#
# Compute FAA and VGG for case of variable density
ps=variablerho.ps
order=4
dx=0.5
# 2 panels of topo and grav, with top profile of admittance & coherence
# NOT FINISHED
# 1. Create a bathymetry data set with one circular plateau
#    with R_base = 50 km, height = 3500 m, depth = -5000 m,
#    and let density contrast to from 1300 at flanks and reach
#    1700 at a radius of 25 km and stay at that through the center.
#    The equivalent average density is 1533.33333
rhoc=1533.333333
echo "0	0	50	3500" | gmt grdseamount -R-256/256/-256/256 -I$dx -r -Cd -Gplateau.nc -Z-5000
gmt grdmath -Rplateau.nc 50 0 0 CDIST SUB 25 DIV 0 MAX 400 MUL 1300 ADD 1700 MIN 0 0 CDIST 50 LT MUL = rho.nc
# BL Plot the bathymetry
gmt grdtrack -Gplateau.nc -ELM/RM -o0,2 -nn > plateau.trk
gmt psxy -R-256/256/-5000/0 -JX5.5i/2.75i -P -K -Ggray -L+yb -BWSn -Baf -Bx+ukm -By+l"Depth (m)" plateau.trk -X1.5i > $ps
echo "-256 0 BATHYMETRY" | gmt pstext -R -J -O -K -F+jTL+f14p,gray -Dj0.1i/0.1i -Gwhite -TO >> $ps
# Plot the variable density profile in red and constant in blue
gmt grdtrack -Grho.nc -ELM/RM -o0,2 -nn > rho.trk
gmt psxy -R-256/256/0/1800 -J -O -K -W1p,red rho.trk -BE -Baf -By+l"Density (kg/m@+3@+)" >> $ps
gmt psxy -R -J -O -K -W1p,blue << EOF >> $ps
-256	0
-50	0
-50	$rhoc
50	$rhoc
50	0
256	0
EOF
echo "+256 1800 CONSTANT DENSITY" | gmt pstext -R -J -O -K -F+jTR+f14p,blue -Dj0.1i/0.1i -Gwhite -TO >> $ps
echo "+256 1800 VARIABLE DENSITY" | gmt pstext -R -J -O -K -F+jTR+f14p,red -Dj0.1i/0.4i -Gwhite -TO >> $ps
# 2a. Compute the VGG anomaly for variable density
gmt gravfft plateau.nc+uk -Drho.nc+uk -Nf+a -Fv -E$order -Gvgg.nc
# BR plot the VGG anomaly
gmt grdtrack -Gvgg.nc -ELM/RM -o0,2 -nn > vgg.trk
gmt psxy -R-256/256/-100/250 -J -O -K -W1p,red vgg.trk -BWSne -Bafg1000 -Bx+ukm -By+l"VGG (E\371tv\371s)" -Y3.2i >> $ps
# 2b. Compute the VGG anomaly for constant density
gmt gravfft plateau.nc+uk -D$rhoc -Nf+a -Fv -E$order -Gvgg.nc
# BR plot the VGG anomaly
gmt grdtrack -Gvgg.nc -ELM/RM -o0,2 -nn > vgg.trk
gmt psxy -R -J -O -K -W1p,blue vgg.trk >> $ps
echo "-256 250 VGG" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
echo "+256 250 CONSTANT DENSITY" | gmt pstext -R -J -O -K -F+jTR+f14p,blue -Dj0.1i/0.1i -Gwhite -TO >> $ps
echo "+256 250 VARIABLE DENSITY" | gmt pstext -R -J -O -K -F+jTR+f14p,red -Dj0.1i/0.4i -Gwhite -TO >> $ps
# 3a. Compute the FAA anomaly for variable density
gmt gravfft plateau.nc+uk -Drho.nc+uk -Nf+a -Ff -E$order -Gfaa.nc
# ML plot the FAA anomaly
gmt grdtrack -Gfaa.nc -ELM/RM -o0,2 -nn > faa.trk
gmt psxy -R-256/256/-25/250 -J -O -K -W1p,red faa.trk -BWSne -Bafg1000 -Bx+ukm -By+l"FAA (mGal)" -Y3.2i >> $ps
# 3a. Compute the FAA anomaly for constant density
gmt gravfft plateau.nc+uk -D$rhoc -Nf+a -Ff -E$order -Gfaa.nc
gmt grdmath faa.nc DUP LOWER SUB = faa.nc
# ML plot the FAA anomaly
gmt grdtrack -Gfaa.nc -ELM/RM -o0,2 -nn > faa.trk
gmt psxy -R -J -O -K -W1p,blue faa.trk >> $ps
echo "-256 250 FAA" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
echo "+256 250 CONSTANT DENSITY" | gmt pstext -R -J -O -K -F+jTR+f14p,blue -Dj0.1i/0.1i -Gwhite -TO >> $ps
echo "+256 250 VARIABLE DENSITY" | gmt pstext -R -J -O -K -F+jTR+f14p,red -Dj0.1i/0.4i -Gwhite -TO >> $ps
gmt psxy -R -J -O -T >> $ps
rm -f vgg.trk faa.trk plateau.trk rho.trk rho.nc plateau.nc vgg.nc faa.nc
