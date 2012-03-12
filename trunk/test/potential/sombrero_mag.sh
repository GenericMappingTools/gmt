#! /bin/bash
# $Id$
#
# Compute the magnetic anomaly of our Mexican hat split in two halves

. functions.sh
header "Compute magnetic anomaly of the Mexican hat"

# Create two half sobreros 
grdmath -R-15/15/-15/15 -I1 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL -5 ADD = sombrero.nc
grdmath -R-15/0/-15/15 -I1 -10 = half_neg.nc
grdmath -R0/15/-15/15  -I1  10 = half_pos.nc
grdpaste half_neg.nc half_pos.nc -Gmag.nc

# Get rough surface description
grd2xyz sombrero.nc -R-15/15/-15/15 > sombrero_xyz.dat

# Calculate the triangles of the two halves
triangulate sombrero_xyz.dat > sombrero_tri.dat

# Now add a fourth column to the xyz files with the magnetization
# which will be constant but have oposite signals for the two halves.
grd2xyz mag.nc -o2 > m.dat 
paste sombrero_xyz.dat m.dat > sombrero_xyzm.dat

# Compute the mag anomaly using a F dec=10,dip=60 & M dec=-10,dip=40. Intensity came from mag.nc grid
xyzokb -Gsombrero_mag.nc -R-15/15/-15/15 -I1.0 -E2 -H10/60/0/-10/40 -Tdsombrero_xyzm.dat/sombrero_tri.dat/m

grd2cpt sombrero_mag.nc -E20 -D > m.cpt
grdimage sombrero_mag.nc -Cm.cpt -JX12c -Ba -P > $ps

pscmp
