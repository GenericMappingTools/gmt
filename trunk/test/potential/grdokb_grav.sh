#! /bin/bash
# $Id: $
#
# Compute the gravity anomaly of the whater layer above the Gorringe bank

ps=grdokb_grav.ps

lim=-12.5/-10/35.5/37.5

# Compute the grav anomaly using a contrast density of 1700 kg/m^3.
# To get the Bouger anomaly this would be added to the FAA
grdokb ../genper/etopo10.nc -R$lim -fg -C1700 -Ggrdokb_grav.nc -I0.05 -Q0.5

# Generate a line to compute a profile
project -C-11.7/37.2 -E-11/35.8 -G1 -Q > tt.xyp

# Compute the anomaly along the tt line
grdokb ../genper/etopo10.nc -R$lim -fg -C1700 -Ftt.xyp -Q0.5 > tt.grv

grdcontour grdokb_grav.nc -C10 -A20 -JM14c -BaWSen -P -K > $ps

# Plot the track
psxy tt.xyp -R -W0.5p -JM -O -K >> $ps

# And computed profile
paste tt.xyp tt.grv | psxy -i2,5 -R0/165/80/360 -JX14c/8c -W1p -BaWS -Y16c -O >> $ps
