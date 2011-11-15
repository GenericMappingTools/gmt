#! /bin/bash
# $Id$
#
# Compute the magnetic anomaly of our mexican hat split in two halves

. ../functions.sh
header "Compute magnetic anomaly of the Mexican hat"

ps=chapeu_mag.ps

# Create two halph sobreros 
grdmath -R-15/15/-15/15 -I1 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL -5 ADD = chapeu.nc
grdmath -R-15/0/-15/15 -I1 -10 = half_neg.nc
grdmath -R0/15/-15/15  -I1  10 = half_pos.nc
grdpaste half_neg.nc half_pos.nc -Gmag.nc

# Get rough surface description
grd2xyz chapeu.nc -R-15/15/-15/15 > chapeu_xyz.dat

# Calculate the triangles of the two halves
triangulate chapeu_xyz.dat > chapeu_tri.dat

# Now add a fourth column to the xyz files with the magnetization
# which will be constant but have oposite signals for the two halves.
grd2xyz mag.nc -o2 > m.dat 
paste chapeu_xyz.dat m.dat > chapeu_xyzm.dat

# Compute the mag anomaly using a F dec=10,dip=60 & M dec=-10,dip=40. Intensity came from mag.nc grid
xyzokb -Gchapeu_mag.nc -R-15/15/-15/15 -I1.0 -E2 -H10/60/0/-10/40 -Tdchapeu_xyzm.dat/chapeu_tri.dat/m

grd2cpt chapeu_mag.nc -E20 -D > m.cpt
grdimage chapeu_mag.nc -Cm.cpt -JX12c -Ba -P > $ps

rm -f half_*.nc mag.nc chapeu_mag.nc chapeu.nc chapeu*.dat m.dat m.cpt

pscmp
