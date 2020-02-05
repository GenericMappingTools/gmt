#!/usr/bin/env bash
#
# Compute approximate flexure over disc, compare to results from Matlab
# using results flex_analytical.txt produced by flex_analytical.m
ps=flexapprox.ps
# Parameters for disc load
gmt set GMT_FFT kiss
h=4000
r=15000
rhom=3300
rhol=2800
rhoi=2400
rhow=1000
Te=10000
# Make disc load
echo 0 0 $r $h | gmt grdseamount -R-511000/511000/-511000/511000 -I2000 -Cd -Gload.nc
# Because the grid representation of a disc is not exact (the disc is built from lots of prisms)
# we compute the ratio of the exact and approximate volumes and adjust the flexure for this difference.
# Do this by counting number of nonzero entries, multiply by prism volumes dx*dx*h
gmt grdmath load.nc 0 NAN = tmp.nc
n_nan=$(gmt grdinfo -M tmp.nc -C | cut -f16)
disc_vol_grid=$(gmt math -Q 512 512 MUL $n_nan SUB 2000 2000 MUL MUL $h MUL =)
disc_vol_exact=$(gmt math -Q $r $r MUL PI MUL $h MUL =)
scale=$(gmt math -Q $disc_vol_exact $disc_vol_grid DIV =)
# Traditional rhoi = rhol
gmt gravfft load.nc -Gflex_a.nc -Q -Nf+l -T$Te/$rhol/$rhom/$rhow
z0=$(echo -511000 -511000 | gmt grdtrack -Gflex_a.nc -o2)
gmt grdmath flex_a.nc $z0 SUB = flex_a.nc
# Approximate rhoi < rhol
gmt gravfft load.nc -Gflex_c.nc -Q -Nf+l -T$Te/$rhol/$rhom/$rhow/$rhoi
z0=$(echo -511000 -511000 | gmt grdtrack -Gflex_c.nc -o2)
gmt grdmath flex_c.nc $z0 SUB = flex_c.nc
gmt grdtrack -Gflex_a.nc+Uk -Gflex_c.nc+Uk -ELM/RM > result.txt
# Plot the exact single-domain case
gmt psxy -R0/256/-1100/50 -JX6i/4i -Xc -P -Bafg10000 @flex_analytical.txt -i0,1 -Sc0.04i -Ggreen -K > $ps
gmt psxy -R -J -O -K result.txt -i0,2+s$scale -W0.25p,red >> $ps
# Plot the exact double-domain data and the approximations
gmt psxy -R -J -O -K -Bafg10000 @flex_analytical.txt -i0,2 -Sc0.04i -Ggreen -Y4.75i >> $ps
gmt psxy -R -J -O -K @flex_analytical.txt -i0,3 -W0.25p >> $ps
gmt psxy -R -J -O -K result.txt -i0,3+s$scale -W0.25p,red,- >> $ps
gmt psxy -R -J -O -T >> $ps
