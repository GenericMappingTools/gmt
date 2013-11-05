#!/bin/bash
ps=profiles.ps
# Test grdtrack -E option for Geo data
gmt grdmath -R0/10/0/10 -I15m -fg X = t.nc
gmt grdtrack -Gt.nc -ELB/TR > a.txt
gmt grdtrack -Gt.nc -E0/10/10/0 > b.txt
gmt grdtrack -Gt.nc -ECM+o30+l800k > c.txt
gmt grdtrack -Gt.nc -ECM+a60+l400k > d.txt
gmt grdtrack -Gt.nc -EBC/TC+i50 > e.txt
gmt grdtrack -Gt.nc -ELM+a90+l800k+n10 > f.txt

gmt psxy -Rt.nc -JM4i -P -Baf -Xc -K a.txt -W1p,red > $ps
gmt psxy -R -J -O -K b.txt -W1p,green >> $ps
gmt psxy -R -J -O -K c.txt -W1p,blue >> $ps
gmt psxy -R -J -O -K d.txt -W1p,yellow >> $ps
gmt psxy -R -J -O -K e.txt -Sc0.1c -Gbrown >> $ps
gmt psxy -R -J -O -K f.txt -Sc0.1c -Gblack >> $ps

# Test grdtrack -E option for Cartesian data
gmt grdmath -R0/10/0/10 -I0.25 X = t.nc
gmt grdtrack -Gt.nc -ELB/TR > a.txt
gmt grdtrack -Gt.nc -E0/10/10/0 > b.txt
gmt grdtrack -Gt.nc -ECM+o30+l8 > c.txt
gmt grdtrack -Gt.nc -ECM+a60+l4 > d.txt
gmt grdtrack -Gt.nc -EBC/TC+i0.5 > e.txt
gmt grdtrack -Gt.nc -ELM+a90+l8+n10 > f.txt

gmt psxy -Rt.nc -JX4i -O -Baf -Y4.75i -K a.txt -W1p,red >> $ps
gmt psxy -R -J -O -K b.txt -W1p,green >> $ps
gmt psxy -R -J -O -K c.txt -W1p,blue >> $ps
gmt psxy -R -J -O -K d.txt -W1p,yellow >> $ps
gmt psxy -R -J -O -K e.txt -Sc0.1c -Gbrown >> $ps
gmt psxy -R -J -O -K f.txt -Sc0.1c -Gblack >> $ps
gmt psxy -R -J -O -T >> $ps
