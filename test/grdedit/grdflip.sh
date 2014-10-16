#!/bin/bash
#	Test grdedits grid rotations
ps=grdflip.ps
# Original grid
gmt grdmath -R-1800/2600/-600/600 -I10 Y 100 DIV 2 POW NEG EXP Y 1000 DIV ADD X 1000 DIV MUL = w.nc
gmt makecpt -Crainbow -T-1.8/2.65/0.05 -Z > t.cpt
gmt grdimage w.nc -Ct.cpt -Jx0.00147727i -Baf -P -K -BWSne -Y8.5i > $ps
# Flip grid left-to-right
gmt grdedit w.nc -Gh.nc -Eh
gmt grdimage h.nc -Ct.cpt -J -Baf -BWSne+tFLIPLR -Y-2.5i -O -K --FONT_TITLE=18p >> $ps
# grid after 180 rotation
gmt grdedit w.nc -Ga.nc -Ea
gmt grdimage a.nc -Ct.cpt -J -O -K -Baf -BWSne+t"180\312" -Y-2.5i --FONT_TITLE=18p >> $ps
# Flip grid top-to-bottom
gmt grdedit w.nc -Gv.nc -Ev
gmt grdimage v.nc -Ct.cpt -J -O -Baf -BWSne+tFLIPUD -Y-2.5i --FONT_TITLE=18p >> $ps
