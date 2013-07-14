#!/bin/bash
ps=profiles.ps
# Test grdtrack -E option for Geo data
grdmath -R0/10/0/10 -I15m -fg X = t.nc
grdtrack -Gt.nc -ELB/TR > a.txt
grdtrack -Gt.nc -E0/10/10/0 > b.txt
grdtrack -Gt.nc -ECM+o30+l800k > c.txt
grdtrack -Gt.nc -ECM+a60+l400k > d.txt
grdtrack -Gt.nc -EBC/TC+i50 > e.txt
grdtrack -Gt.nc -ELM+a90+l800k+n10 > f.txt

psxy -Rt.nc -JM4i -P -Baf -Xc -K a.txt -W1p,red > $ps
psxy -R -J -O -K b.txt -W1p,green >> $ps
psxy -R -J -O -K c.txt -W1p,blue >> $ps
psxy -R -J -O -K d.txt -W1p,yellow >> $ps
psxy -R -J -O -K e.txt -Sc0.1c -Gbrown >> $ps
psxy -R -J -O -K f.txt -Sc0.1c -Gblack >> $ps

# Test grdtrack -E option for Cartesian data
grdmath -R0/10/0/10 -I0.25 X = t.nc
grdtrack -Gt.nc -ELB/TR > a.txt
grdtrack -Gt.nc -E0/10/10/0 > b.txt
grdtrack -Gt.nc -ECM+o30+l8 > c.txt
grdtrack -Gt.nc -ECM+a60+l4 > d.txt
grdtrack -Gt.nc -EBC/TC+i0.5 > e.txt
grdtrack -Gt.nc -ELM+a90+l8+n10 > f.txt

psxy -Rt.nc -JX4i -O -Baf -Y4.75i -K a.txt -W1p,red >> $ps
psxy -R -J -O -K b.txt -W1p,green >> $ps
psxy -R -J -O -K c.txt -W1p,blue >> $ps
psxy -R -J -O -K d.txt -W1p,yellow >> $ps
psxy -R -J -O -K e.txt -Sc0.1c -Gbrown >> $ps
psxy -R -J -O -K f.txt -Sc0.1c -Gblack >> $ps
psxy -R -J -O -T >> $ps
