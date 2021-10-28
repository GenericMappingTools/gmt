#!/usr/bin/env bash
# Drape image via three r,g,b grids on top of 3-D topo relief
# testing both old and new syntax
ps=rgb.ps
gmt grdmath -R0/6/0/6 -I0.1 X 6 DIV 255 MUL = r.nc
gmt grdmath -R0/6/0/6 -I0.1 Y 6 DIV 255 MUL = g.nc
gmt grdmath -R0/6/0/6 -I0.1 3 3 CDIST DUP UPPER DIV 255 MUL  = b.nc
gmt grdcut @earth_relief_06m -R0/6/0/6 -Gtopo.nc
gmt grdview topo.nc -I+ -JM4i -JZ2i -p145/35 -Gr.nc,g.nc,b.nc -Baf -BWSne -Qi100i -P -K -X1.5i -Y0.75i > $ps
gmt grdview topo.nc -I+ -J -JZ -p -Gr.nc -Gg.nc -Gb.nc -Baf -BWSne -Qi100i -O -Y4.75i >> $ps
