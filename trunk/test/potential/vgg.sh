#!/bin/sh
#	$Id$
#
# Compute Vertical Gravity Gradient (VGG) for synthetic seamount
ps=vgg.ps
n=4
# 2 panels of topo and grav, with top profile of admittance & coherence
# NOT FINISHED
# 1. Create a bathymetry data set with one circular truncated seamount
#    as in Fig 3. of Marks & Smith, 2007 [GRL].
echo "0	0	25	3751" | grdseamount -R-256/256/-256/256 -I1 -r -C -Gsmt.nc -T0.4 -Z-5084
# BL Plot the bathymetry
makecpt -Crainbow -T-5100/-1000/200 -Z > t.cpt
grdimage smt.nc -R-100/100/-100/100 -JX3i -P -BagWSne -Ct.cpt -K > $ps
grdtrack -Gsmt.nc -ELM/RM > smt.trk
psxy -R -J -O -K -W1p smt.trk >> $ps
# 2. Compute the FAA anomaly
#gravfft smt.nc+uk -D1700 -L -Nf -Fv -E$n -Gfaa.nc
grdtrack -Gfaa.nc -ELM/RM > faa.trk
psxy -R -J -O -K -W1p faa.trk >> $ps
# BR plot the FAA anomaly
makecpt -Crainbow -T-300/300/25 -Z > t.cpt
grdimage faa.nc -R-100/100/-100/100 -JX3i -O -BagwSne -Ct.cpt -K -X3.5i >> $ps
# 3. Compute the VGG anomaly
#gravfft smt.nc+uk -D1700 -L -Nf -Fv -E$n -Gvgg.nc
# ML plot the VGG anomaly
makecpt -Crainbow -T-400/400/25 -Z > t.cpt
grdimage vgg.nc -R-100/100/-100/100 -JX3i -O -BagwSne -Ct.cpt -K -X-3.5i -Y3.25i >> $ps
grdtrack -Gvgg.nc -ELM/RM > vgg.trk
psxy -R -J -O -K -W1p vgg.trk >> $ps
# 4. Compute the geoid anomaly
#gravfft smt.nc+uk -D1700 -L -Nf -Fg -E$n -Ggeoid.nc
# MR plot the VGG anomaly
makecpt -Crainbow -T0/5/0.25 -Z > t.cpt
grdimage geoid.nc -R-100/100/-100/100 -JX3i -O -BagwSne -Ct.cpt -K -X3.5i >> $ps
grdtrack -Ggeoid.nc -ELM/RM > geoid.trk
psxy -R -J -O -K -W1p geoid.trk >> $ps
# 5 Plot crossections of bathy and faa crossections
# TL plot the bathy and faa canomaly
psxy -R-100/100/-5100/-1200 -JX3i/2.5i -O -K -W1p -i0,3 smt.trk -BafWsN -X-3.5i -Y3.2i >> $ps
psxy -R-100/100/-400/400 -J -O -K -W0.25p,red -i0,3 faa.trk -BafsEN >> $ps
# Add VGG and geoid crossections
# TRL plot the VGG and geoid anomaly
psxy -R-100/100/-400/400 -JX3i/2.5i -O -K -W1p,blue -i0,3 vgg.trk -Bafg1000wsNe -X3.5i >> $ps
psxy -R-100/100/0/5 -J -O -K -W0.25p,green -i0,3 geoid.trk >> $ps
psxy -R -J -O -T >> $ps
