#!/bin/sh
#	$Id$
#
# Compute FAA, VGG, and geoid over synthetic seamount
ps=fields.ps
order=2
# 2 panels of topo and grav, with top profile of admittance & coherence
# NOT FINISHED
# 1. Create a bathymetry data set with one circular truncated seamount
#    as in Fig 3. of Marks & Smith, 2007 [GRL], with R_base = 35 km,
# R_top = 10 km, height = 3751 m, depth = -5084 m, density d_rho = 2800-1030
# = 1670 kg/m^3, so the flattening is 10/25 = 0.4.
echo "0	0	25	3751" | grdseamount -R-256/256/-256/256 -I1 -r -C -Gsmt.nc -T0.4 -Z-5084
# BL Plot the bathymetry
makecpt -Crainbow -T-5100/-1000/200 -Z > t.cpt
grdimage smt.nc -R-100/100/-100/100 -JX3i -P -BagWSne -Ct.cpt -K > $ps
grdtrack -Gsmt.nc -ELM/RM > smt.trk
psxy -R -J -O -K -W5p,white smt.trk >> $ps
psxy -R -J -O -K -W1p smt.trk >> $ps
echo "-100 100 BATHYMETRY" | pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 2. Compute the VGG anomaly
gravfft smt.nc+uk -D1670 -Nf -Fv -E$order -Gvgg.nc
# BR plot the VGG anomaly
makecpt -Crainbow -T-50/250/25 -Z > t.cpt
grdimage vgg.nc -R-100/100/-100/100 -JX3i -O -BagwSne -Ct.cpt -K -X3.5i >> $ps
grdtrack -Gvgg.nc -ELM/RM > vgg.trk
psxy -R -J -O -K -W5p,white vgg.trk >> $ps
psxy -R -J -O -K -W1p,blue vgg.trk >> $ps
echo "-100 100 VGG" | pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 3. Compute the FAA anomaly
gravfft smt.nc+uk -D1670 -Nf -Ff -E$order -Gfaa.nc
# ML plot the FAA anomaly
makecpt -Crainbow -T-50/250/25 -Z > t.cpt
grdimage faa.nc -R-100/100/-100/100 -JX3i -O -BagWsne -Ct.cpt -K -X-3.5i -Y3.25i >> $ps
grdtrack -Gfaa.nc -ELM/RM > faa.trk
psxy -R -J -O -K -W5p,white faa.trk >> $ps
psxy -R -J -O -K -W1p,red faa.trk >> $ps
echo "-100 100 FAA" | pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 4. Compute the geoid anomaly
gravfft smt.nc+uk -D1670 -Nf -Fg -E$order -Ggeoid.nc
# MR plot the VGG anomaly
makecpt -Crainbow -T0/5/0.25 -Z > t.cpt
grdimage geoid.nc -R-100/100/-100/100 -JX3i -O -Bagwsne -Ct.cpt -K -X3.5i >> $ps
grdtrack -Ggeoid.nc -ELM/RM > geoid.trk
psxy -R -J -O -K -W5p,white geoid.trk >> $ps
psxy -R -J -O -K -W1p,orange geoid.trk >> $ps
echo "-100 100 GEOID" | pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 5 Plot crossections of bathy and faa crossections
# TL plot the bathy and faa canomaly
psxy -R-100/100/-5100/1000 -JX3i/2.5i -O -K -W1p -i0,3 smt.trk -BafWsN -X-3.5i -Y3.2i >> $ps
echo "-100 1000 TOPO" | pstext -R -J -O -K -F+jTL+f12p -Dj0.1i/0.1i >> $ps
psxy -R-100/100/-50/250 -J -O -K -W1p,red -i0,3 faa.trk -Bafg1000ENs >> $ps
echo "100 250 FAA" | pstext -R -J -O -K -F+jTR+f12p,Helvetica,red -Dj0.1i/0.1i >> $ps
# Add VGG and geoid crossections
# TRL plot the VGG and geoid anomaly
psxy -R-100/100/-50/250 -JX3i/2.5i -O -K -W1p,blue -i0,3 vgg.trk -Bafg1000wsN -X3.5i >> $ps
echo "-100 250 VGG" | pstext -R -J -O -K -F+jTL+f12p,Helvetica,blue -Dj0.1i/0.1i >> $ps
psxy -R-100/100/0/4 -J -O -K -W1p,orange -i0,3 geoid.trk -BafE >> $ps
echo "100 4 GEOID" | pstext -R -J -O -K -F+jTR+f12p,Helvetica,orange -Dj0.1i/0.1i >> $ps
psxy -R -J -O -T >> $ps
