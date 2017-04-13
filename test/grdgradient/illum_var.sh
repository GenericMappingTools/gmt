#!/bin/bash
# $Id$
# Test the generation of illumination for variable azimuth

ps=illum_var.ps

gmt grdmath -R-15/15/-15/15 -I0.1 Y 30 MUL SIND X 30 MUL SIND MUL = sin.nc
gmt grdmath -R-15/15/-15/15 -I0.1 0 10 CAZ = az.nc

gmt grd2cpt sin.nc -Cjet > pal.cpt
gmt grd2cpt az.nc  -Cjet > az.cpt

gmt grdgradient sin.nc -Aaz.nc -Gintensity.nc -Nt1.25

gmt grdcontour az.nc -JX4.5i -Baf -C10 -A30 -K -P -Zp -GlLM/RM -Xc > $ps
gmt grdimage sin.nc -J -Baf -BWsNE -Iintensity.nc -Cpal.cpt -O -Y5i >> $ps

rm -f sin.nc intensity.nc pal.cpt
