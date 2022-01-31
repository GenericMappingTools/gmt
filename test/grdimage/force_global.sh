#!/usr/bin/env bash
#
# 6 different global grids specs - supposedly all covering the planet but
# often have trouble saying that in the w/e/s/n specs.
# DVC_TEST
ps=force_global.ps
gmt grdmath -Rg -I30m X COSD Y COSD MUL = xy0g.grd
gmt grdmath -R0/359:30/-89:45/89:45 -I30m  X COSD Y COSD MUL = xy1g.grd
gmt grdmath -R0/359:30/-90/90 -I30m X COSD Y COSD MUL = xy2g.grd
gmt grdmath -R0:15/359:45/-89:45/89:45 -I30m  X COSD Y COSD MUL = xy3g.grd
gmt grdmath -Rg -I30m -r X COSD Y COSD MUL = xy0p.grd
gmt grdmath -R0:30/360:30/-90/90 -I30m -r X COSD Y COSD MUL = xy1p.grd
gmt makecpt -Cpolar -T-1/1 > t.cpt
gmt grdimage xy0g.grd -JH0/3.5i -P -K -Ct.cpt -X0.5i -B+t"-Rg" > $ps
gmt grdimage xy1g.grd -J -O -K -Ct.cpt -X3.75i -B+t"-R0/359:30/-89:45/89:45" >> $ps
gmt grdimage xy2g.grd -J -O -K -Ct.cpt -X-3.75i -Y3i -B+t"-R0/359:30/-90/90" >> $ps
gmt grdimage xy1g.grd -J -O -K -Ct.cpt -X3.75i -B+t"-R0:15/359:45/-89:45/89:45" >> $ps
gmt grdimage xy0p.grd -J -O -K -Ct.cpt -X-3.75i -Y3i -B+t"-Rg -r" >> $ps
gmt grdimage xy0p.grd -J -O -Ct.cpt -X3.75i -B+t"-R0:30/360:30/-90/90 -r" >> $ps
