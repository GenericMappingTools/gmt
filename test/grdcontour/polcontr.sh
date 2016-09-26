#!/bin/bash
# Test based on issue # 968.  This one uses -JP6ir and fails.
ps=polcontr.ps
gmt grd2xyz -s "${src:-.}"/test.dat.nc > t.txt
gmt psxy -R"${src:-.}"/test.dat.nc -JP6ir t.txt -Sc0.05c -By30 -Bx30 -BWSnE -C"${src:-.}"/test.dat.cpt -K -P > $ps
gmt grdcontour "${src:-.}"/test.dat.nc -J -C"${src:-.}"/test.dat.cpt -A- -W+1p -By30 -Bx30 -BWSnE -O -Y4i >> $ps
