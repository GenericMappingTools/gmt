#! /bin/bash
# $Id$
#
# Compute the magnetic anomaly of our Mexican hat split in two halves

ps=sombrero_mag.ps

# Create two half sobreros 
gmt grdmath -R-15/15/-15/15 -I1 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL -5 ADD = sombrero.nc
gmt grdmath -R-15/0/-15/15 -I1 -10 = half_neg.nc
gmt grdmath -R0/15/-15/15  -I1  10 = half_pos.nc
gmt grdpaste half_neg.nc half_pos.nc -Gmag.nc

# Get rough gmt surface description
gmt grd2xyz sombrero.nc -R-15/15/-15/15 > sombrero_xyz.dat

# Calculate the triangles of the two halves
gmt triangulate sombrero_xyz.dat > sombrero_tri.dat

# Now add a fourth column to the xyz files with the magnetization
# which will be constant but have oposite signals for the two halves.
gmt grd2xyz mag.nc -o2 > m.dat 
paste sombrero_xyz.dat m.dat > sombrero_xyzm.dat

# Compute the mag anomaly using a F dec=10,dip=60 & M dec=-10,dip=40. Intensity came from mag.nc grid
gmt gmtgravmag3d -Gsombrero_mag.nc -R-15/15/-15/15 -I1.0 -E2 -H10/60/0/-10/40 -Tdsombrero_xyzm.dat/sombrero_tri.dat/m

gmt grd2cpt sombrero_mag.nc -E20 -D > m.cpt
gmt grdimage sombrero_mag.nc -Cm.cpt -JX12c -Ba -P > $ps

