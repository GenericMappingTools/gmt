#! /bin/bash
# $Id$
#
# Compute the gravity anomaly of the Flemish Cap Guyot

ps=fc_okb.ps

lim=-41:50/-41:20/47:30/47:50 

# Get rough gmt surface description
gmt grd2xyz ../genper/etopo10.nc -R$lim -fg > fc.dat

# Calculate the triangles
gmt triangulate fc.dat > fc_tri.dat

# Compute the grav anomaly using a contras density of 1700 kg/m^3
gmt gmtgravmag3d -C1700 -Gfc_okb.nc -R -I1m -Z-4300 -fg -Tdfc.dat/fc_tri.dat

gmt grdcontour fc_okb.nc -C2.5 -A5 -JM14c -Ba -BWSen -P > $ps

