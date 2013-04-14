#!/bin/sh
#
#       $Id$

# Test surface with periodic boundary conditions in longitude
ps=periodic.ps

# Make global synthetic grid
grdmath -Rg -I1 X SIND Y 3 MUL COSD MUL 100 MUL = t.nc
gmtmath -T-45/45/1 0 = path.txt
# Sample this at ~200 places randomly; save random in svn sp repeatable
#gmtmath -T0/200/1 -o1 0 360 RAND = x
#gmtmath -T0/200/1 -o1 -90 90 RAND = y
#paste x y > random_xy.txt
# Sample synthetic grid at random locations
grdtrack -Gt.nc random_xy.txt | blockmean -Rg -I1 -fg > data.txt
# Grid with surface so periodic boundaries are 0|360
surface -Rg -I1 -fg data.txt -Gdatag.nc
# Grid with surface so periodic boundaries are -180/+180
surface -Rd -I1 -fg data.txt -Gdatad.nc
# Contour grid where Greenwhich was in the middle when gridding
grdcontour datad.nc -C10 -A50 -JQ0/5.5i -P -BafWSne -K -Xc -Y0.5i > $ps
psxy -R -J -O -K path.txt -W1p,green >> $ps
psxy -R -J -O -K data.txt -Ss0.05i -Gred >> $ps
# Contour grid where Dateline was in the middle when gridding
grdcontour datag.nc -C10 -A50 -JQ0/5.5i -O -BafWsne -K -Y3i >> $ps
psxy -R -J -O -K path.txt -W1p,blue >> $ps
psxy -R -J -O -K data.txt -Ss0.05i -Gred >> $ps
# Original synthetic grid for comparison
grdcontour t.nc -C10 -A50 -JQ0/5.5i -O -BafWsne -K -Y3i >> $ps
# Plot crossection along Equator
grdtrack path.txt -Gdatad.nc -o0,2 | psxy -R-45/45/-70/70 -JX5.5i/1i -Y3i -Bafg90Wsne -W2p,green -O -K >> $ps
grdtrack path.txt -Rd -Gdatag.nc -o0,2 | psxy -R-45/45/-70/70 -J -W0.5p,blue -O -K >> $ps
psxy -R -J -O -T >> $ps
