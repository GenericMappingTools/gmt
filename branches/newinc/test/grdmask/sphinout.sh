#!/bin/bash
#       $Id$
#
# Test gmt grdmask for proper handling of inside/outside for spherical polygons

ps=sphinout.ps

# 2k.dat contains many repeat points but no point are integer lon,lat
# hence, no node in the grid below lies exactly on the perimeter. We
# therefore should only see read and green colors in the image.
cat << EOF > mask.cpt
-0.25   red     0.25    red	; OUTSIDE
0.25    yellow  0.75    yellow	; ON LINE
0.75    green   1.25    green	; INSIDE
EOF
gmt grdmask 2k.dat -Gmask.nc -N0/0.5/1 -I1 -Rg -fg -A
gmt grdimage mask.nc -Jx0.015id -Cmask.cpt -Bx60 -By30 -BWSne -K -P -Xc -Y1.5i > $ps
gmt psxy -Rmask.nc -J -O -K 2k.dat -W0.25p,blue >> $ps
gmt psscale -Cmask.cpt -L0.1i -D2.7i/-0.5i/3i/0.1ih -O -K >> $ps
gmt grdimage mask.nc -R295/345/59/82 -Jx0.108i -Cmask.cpt -B10g1 -BWSne -O -K -Y3.1i >> $ps
gmt psxy -R -J -O -K 2k.dat -W0.5p,blue >> $ps
# Now we truncate the coordinates to be all integers, meaning the mask grid
# should have a yellow halo from all those nodes that lie on the perimeter.
gmt gmtmath -T 2k.dat FLOOR = tmp.txt
gmt grdmask -Gmask.nc -N0/0.5/1 -I1 -Rg -fg tmp.txt -A
gmt grdimage mask.nc -R295/345/59/82 -Jx0.108i -Cmask.cpt -B10g1 -BWSne -O -K -Y2.9i >> $ps
gmt psxy -R -J -O tmp.txt -W0.5p,blue >> $ps

