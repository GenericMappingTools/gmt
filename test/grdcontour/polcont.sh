#!/bin/bash
# Test based on issue # 968.  This one uses -JP6i and works
# polcontr.sh uses -JP6ir and failed.
ps=polcont.ps
gmt grd2xyz -s test.dat.nc > t.txt
psxy -Rtest.dat.nc -JP6i t.txt -Sc0.05c -By30 -Bx30 -BWSnE -Ctest.dat.cpt -K -P > $ps
gmt grdcontour test.dat.nc -J -Ctest.dat.cpt -A- -W+1p -By30 -Bx30 -BWSnE -O -Y4i >> $ps
