#! /bin/bash
# $Id$
#
# Compute the gravity anomaly of the Flemish Cap Guyot

. functions.sh
header "Compute gravity anom of a seamount with Okabe method"

ps=fc_okb.ps
lim=-41:50/-41:20/47:30/47:50 

# Get rough surface description
grd2xyz $src/../genper/etopo10.nc -R$lim -fg > fc.dat

# Calculate the triangles
triangulate fc.dat > fc_tri.dat

# Compute the grav anomaly using a contras density of 1700 kg/m^3
xyzokb -C1700 -Gfc_okb.nc -R -I1m -Z-4300 -M -Tdfc.dat/fc_tri.dat

grdcontour fc_okb.nc -C2.5 -A5 -JM14c -BaWSen -P > $ps

pscmp
