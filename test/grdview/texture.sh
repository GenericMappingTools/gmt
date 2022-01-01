#!/usr/bin/env bash
# Drape a texture image on top of 3-D topo relief
ps=texture.ps
gmt grdcut @earth_relief_01m -R0/10/0/10 -Gtopo.nc
gmt grdview topo.nc -I+nt0.5 -JM5i -JZ2i -p145/35 -G@wood_texture.jpg -Baf -BWSne -Qi100i -P -K -Y0.75i > $ps
gmt grdview topo.nc -JM3i -G@wood_texture.jpg -Qi100i -O -K -Y6i >> $ps
gmt psimage @wood_texture.jpg -Dx0/0+w3i -X3.5i -O >> $ps
