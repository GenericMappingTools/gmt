#!/usr/bin/env bash
#
# Compute the gravity anomaly of the whater layer above the Gorringe bank

ps=grdokb_grav.ps

lim=-12.5/-10/35.5/37.5

# Compute the grav anomaly using a contrast density of 1700 kg/m^3.
# To get the Bouger anomaly this would be added to the FAA
gmt grdgravmag3d @earth_relief_10m -R$lim -fg -C1700 -Ggrdokb_grav.nc -I0.05 -Q0.5

# Generate a line to compute a profile
gmt project -C-11.7/37.2 -E-11/35.8 -G1 -Q > tt.xyp

# Compute the anomaly along the tt line
gmt grdgravmag3d @earth_relief_10m -R$lim -fg -C1700 -Ftt.xyp -Q0.5 > tt.grv

gmt grdcontour grdokb_grav.nc -C10 -A20 -JM14c -Ba -BWSen -P -K > $ps

# Plot the track
gmt psxy tt.xyp -R -W0.5p -JM -O -K >> $ps

# And computed profile
paste tt.xyp tt.grv | gmt psxy -i2,5 -R0/165/80/360 -JX14c/8c -W1p -Ba -BWS -Y16c -O >> $ps
