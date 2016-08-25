#!/bin/bash
# Test grd2rgb with color table
ps=channels.ps
gmt makecpt -Crainbow -T700/1000/50 -Z > t.cpt
gmt makecpt -Cgray -T0/255/1 -Z > g.cpt
gmt grd2rgb hill.nc -Ct.cpt -Ghill_%c.nc
gmt grdimage hill.nc -JX3i -P -Baf -BWSne -K -Ct.cpt > $ps
gmt grdimage hill_r.nc -J -O -Baf -BwSnE -K -Cg.cpt -X3.25i >> $ps
gmt grdimage hill_g.nc -J -O -Baf -BWSne -K -Cg.cpt -X-3.25i -Y3.5i >> $ps
gmt grdimage hill_b.nc -J -O -Baf -BwSnE -Cg.cpt -X3.25i >> $ps
