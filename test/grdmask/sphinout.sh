#!/bin/bash
#       $Id$
#
# Test grdmask for proper handling of inside/outside for spherical polygons

. ../functions.sh
header "Test grdmask with spherical polygon in/on/out"

# 2k.dat contains many repeat points but no point are integer lon,lat
# hence, no node in the grid below lies exactly on the perimeter. We
# therefore should only see read and green colors in the image.
ps=sphinout.ps
cat << EOF > mask.cpt
-0.25   red     0.25    red	; OUTSIDE
0.25    yellow  0.75    yellow	; ON LINE
0.75    green   1.25    green	; INSIDE
EOF
grdmask 2k.dat -Gmask.nc -N0/0.5/1 -I1 -Rg -fg -A
grdimage mask.nc -Jx0.015id -Cmask.cpt -B60/30WSne -K -P -Xc -Y1.5i > $ps
psxy -Rmask.nc -J -O -K 2k.dat -W0.25p,blue >> $ps
psscale -Cmask.cpt -L0.1i -D2.7i/-0.5i/3i/0.1ih -O -K >> $ps
grdimage mask.nc -R295/345/59/82 -Jx0.108i -Cmask.cpt -B10g1WSne -O -K -Y3.1i >> $ps
psxy -R -J -O -K 2k.dat -W0.5p,blue >> $ps
# Now we truncate the coordinates to be all integers, meaning the mask grid
# should have a yellow halo from all those nodes that lie on the perimeter.
gmtmath -T 2k.dat FLOOR = tmp.txt
grdmask -Gmask.nc -N0/0.5/1 -I1 -Rg -fg tmp.txt -A
grdimage mask.nc -R295/345/59/82 -Jx0.108i -Cmask.cpt -B10g1WSne -O -K -Y2.9i >> $ps
psxy -R -J -O tmp.txt -W0.5p,blue >> $ps

rm -f mask.cpt mask.nc tmp.txt

pscmp
