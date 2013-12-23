#!/bin/bash
ps=profiles.ps
# Test grdtrack -E option for Geo data
gmt grdmath -R0/10/0/10 -I15m -fg X = t.nc
gmt grdtrack -Gt.nc -ELB/TR+d > a.txt
gmt grdtrack -Gt.nc -E0/10/10/0+d > b.txt
gmt grdtrack -Gt.nc -ECM+o30+l800k+d > c.txt
gmt grdtrack -Gt.nc -ECM+a60+l400k+d > d.txt
gmt grdtrack -Gt.nc -EBC/TC+i50+d > e.txt
gmt grdtrack -Gt.nc -ELM+a90+l800k+n10+d > f.txt

gmt psxy -Rt.nc -JM4i -P -Baf -Xc -K a.txt -W1p,red > $ps
gmt psxy -R -J -O -K b.txt -W1p,green >> $ps
gmt psxy -R -J -O -K c.txt -W1p,blue >> $ps
gmt psxy -R -J -O -K d.txt -W1p,yellow >> $ps
gmt psxy -R -J -O -K e.txt -Sc0.1c -Gbrown >> $ps
gmt psxy -R -J -O -K f.txt -Sc0.1c -Gblack >> $ps

# Test grdtrack -E option for Cartesian data
gmt grdmath -R0/10/0/10 -I0.25 X = t.nc
gmt grdtrack -Gt.nc -ELB/TR+d > a.txt
gmt grdtrack -Gt.nc -E0/10/10/0+d > b.txt
gmt grdtrack -Gt.nc -ECM+o30+l8+d > c.txt
gmt grdtrack -Gt.nc -ECM+a60+l4+d > d.txt
gmt grdtrack -Gt.nc -EBC/TC+i0.5+d > e.txt
gmt grdtrack -Gt.nc -ELM+a90+l8+n10+d > f.txt

gmt psxy -Rt.nc -JX4i -O -Baf -Y4.75i -K a.txt -W1p,red >> $ps
gmt psxy -R -J -O -K b.txt -W1p,green >> $ps
gmt psxy -R -J -O -K c.txt -W1p,blue >> $ps
gmt psxy -R -J -O -K d.txt -W1p,yellow >> $ps
gmt psxy -R -J -O -K e.txt -Sc0.1c -Gbrown >> $ps
gmt psxy -R -J -O -K f.txt -Sc0.1c -Gblack >> $ps
gmt psxy -R -J -O -T >> $ps
