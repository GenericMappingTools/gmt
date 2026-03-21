#!/usr/bin/env bash
#
# Test Gouraud shading for grdview surfaces with -Qg
# Creates comparison plots: flat shading vs Gouraud shading

ps=gouraud.ps

# Create a simple synthetic grid (cone/peak)
gmt grdmath -R-5/5/-5/5 -I0.2 X Y HYPOT NEG 3 POW 10 MUL = peak.nc

# Create CPT
gmt makecpt -Chot -T-3536/0 > t.cpt

# Row 1: Flat shading (traditional)
gmt grdview peak.nc -R-5/5/-5/5/-3536/0 -JX8c -JZ4c -Qs -Ct.cpt -p135/30 \
    -Bxafg -Byafg -Bzafg -BWSneZ+t"Flat Shading (-Qs)" -P -K -Xc -Y18c > $ps

# Row 2: Gouraud shading (default diagonal)
gmt grdview peak.nc -R-5/5/-5/5/-3536/0 -JX8c -JZ4c -Qg -Ct.cpt -p135/30 \
    -Bxafg -Byafg -Bzafg -BWSneZ+t"Gouraud (-Qg)" -O -K -Y-10c >> $ps

# Row 3: Gouraud with mesh
gmt grdview peak.nc -R-5/5/-5/5/-3536/0 -JX8c -JZ4c -Qgm -Ct.cpt -p135/30 \
    -Bxafg -Byafg -Bzafg -BWSneZ+t"Gouraud with mesh (-Qgm)" -O -Y-10c >> $ps

# Cleanup
rm -f peak.nc t.cpt
