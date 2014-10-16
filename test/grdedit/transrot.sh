#!/bin/bash
#	Test grdedits grid transpose and rotations
ps=transrot.ps
# Original grid
grdmath -R-1800/2600/-600/600 -I10 Y 100 DIV 2 POW NEG EXP Y 1000 DIV ADD X 1000 DIV MUL = w.nc
makecpt -Crainbow -T-1.8/2.65/0.05 -Z > t.cpt
grdimage w.nc -Ct.cpt -Jx0.00147727i -Baf -P -K -BWSne -Y8.5i --PS_MEDIA=letter > $ps
# Transpose grid
grdedit w.nc -Gt.nc -Et -V
grdimage t.nc -Ct.cpt -J -Baf -BWSne+tTRANSPOSE -Y-7.5i -O -K >> $ps
# grid after 90 CCW rotation
grdedit w.nc -Gp.nc -E+r -V
grdimage p.nc -Ct.cpt -J -O -K -Baf -BWSne+t"90\232 CCW" -X2.363632i >> $ps
# grid after 90 CW rotation
grdedit w.nc -Gm.nc -E-r -V
grdimage m.nc -Ct.cpt -J -O -Baf -BWSne+t"90\232 CW" -X2.363632i >> $ps
