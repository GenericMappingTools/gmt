#!/usr/bin/env bash
# Drape a texture image on top of 3-D topo relief. Once with a plain image and another with a georeferenced one. 

gmt begin texture2_modern ps
	gmt grdcut @earth_relief_01m_p -R0/10/0/10 -Gtopo.nc
	gmt grdview topo.nc -I+nt0.5 -JM10c -JZ4c -p145/35 -G@wood_texture.jpg -Baf -BWSne -Qi100
	gmt grdview topo.nc -I+nt0.5 -JM10c -JZ4c -p145/35 -G@earth_day_01m -Baf -BWSne -Qi100 -Y10.0c
gmt end show
