#!/usr/bin/env bash
#

# Test gmt surface with periodic boundary conditions in longitude
ps=periodic.ps

G=-Gl-90/90/-90/-90,90/90/90/-90
# Make global synthetic grid
gmt grdmath -Rg -I1 X SIND Y 3 MUL COSD MUL 99 MUL = t.nc
gmt math -T-45/45/1 0 = path.txt
# Sample this at ~200 places randomly; saved random in GitHub so repeatable
#gmt math -T0/200/1 -o1 0 360 RAND = x
#gmt math -T0/200/1 -o1 -90 90 RAND = y
#paste x y > random_xy.txt
# Sample synthetic grid at random locations
gmt grdtrack -Gt.nc random_xy.txt | gmt blockmean -Rg -I1 -fg > data.txt
# Grid with gmt surface so periodic boundaries are 0|360
gmt surface -R0/360/-85/85 -I1 -fg data.txt -Gdatag.nc
# Grid with gmt surface so periodic boundaries are -180/+180
gmt surface -R-180/180/-85/85 -I1 -fg data.txt -Gdatad.nc
# Contour grid where Greenwhich was in the middle when gridding
gmt grdcontour datad.nc -C10 -A50 -JQ0/5.5i -P $G -Baf -BWSne -K -Xc -Y0.65i > $ps
gmt psxy -R -J -O -K path.txt -W1p,green >> $ps
gmt psxy -R -J -O -K data.txt -Ss0.05i -Gred >> $ps
# Contour grid where Dateline was in the middle when gridding
gmt grdcontour datag.nc -C10 -A50 -JQ0/5.5i $G -O -Baf -BWsne -K -Y2.85i >> $ps
gmt psxy -R -J -O -K path.txt -W1p,blue >> $ps
gmt psxy -R -J -O -K data.txt -Ss0.05i -Gred >> $ps
# Original synthetic grid for comparison
gmt grdcontour t.nc -C10 -A50 -JQ0/5.5i $G -O -Baf -BWsne -K -Y2.85i >> $ps
# Plot crossection along Equator
gmt grdtrack path.txt -Gdatad.nc -o0,2 | gmt psxy -R-45/45/-70/70 -JX5.5i/1i -Y3i -Bafg90 -BWsne -W2p,green -O -K >> $ps
gmt grdtrack path.txt -Rd -Gdatag.nc -o0,2 | gmt psxy -R-45/45/-70/70 -J -W0.5p,blue -O >> $ps
