#!/bin/bash
#	$Id$
#
# Compute approximate flexure over disc, compare to results from Matlab
# using results flex_analytical.txt produced by flex_analytical.m
ps=flexapprox.ps
# Parameters for disc load
h=4000
r=15000
rhom=3300
rhol=2800
rhoi=2400
rhow=1000
Te=10
# Make disc load
echo 0 0 $r $h | gmt grdseamount -R-511000/511000/-511000/511000 -I2000 -Cd -T0.4 -Z-5000 -Gload.nc
# Traditional rhoi = rhol
gmt gravfft load.nc -T$Te/$rhol/$rho_m/$rho_w        -Gflex_a.nc -Nf+l
# Approximate rhoi < rhol
gmt gravfft load.nc -T$Te/$rhol/$rho_m/$rho_w/$rho_i -Gflex_c.nc -Nf+l
gmt grdtrack -Gflex_a.nc+uk -Gflex_c.nc+uk -ELM/RM > result.txt
# Plot the exact single-domain case
psxy -R0/512/-1000/100 -JX6i/4i -Xc -P -Baf flex_analytical.txt -i0,1 -S0.1i -Ggreen -K > $ps
psxy -R -J -O -K result.txt -i0,2 -W0.25p >> $ps
# Plot the exact double-domain data and the approximations
psxy -R -J -O -K -Baf flex_analytical.txt -i0,2 -S0.1i -Ggreen -Y4.5i >> $ps
psxy -R -J -O -K flex_analytical.txt -i0,3 -W0.25p,blue >> $ps
psxy -R -J -O -K result.txt -i0,3 -W0.25p,red >> $ps
psxy -R -J -O -T >> $ps
