#!/usr/bin/env bash
# Test based on issue # 968.  This one uses -JP6i+re and fails.
ps=polcontr.ps
gmt grd2xyz -s @test.dat.nc > t.txt
gmt psxy -R@test.dat.nc -JP6i+fe t.txt -Sc0.05c -By30 -Bx30 -BWSnE -C@test.dat.cpt -K -P > $ps
gmt grdcontour @test.dat.nc -J -C@test.dat.cpt -An -W1p+cl -By30 -Bx30 -BWSnE -O -Y4i >> $ps
