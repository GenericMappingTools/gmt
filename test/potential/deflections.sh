#!/bin/sh
#	$Id$
#
# Compute E-W and N-S deflections over synthetic seamount
ps=deflections.ps
order=2
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
# 2. Compute the E-W deflection anomaly
gravfft smt.nc+uk -D1670 -Nf -Fe -E$order -Gdef_e.nc
# ML plot the E-W deflection anomaly
makecpt -Cpolar -T-120/120/20 -Z > t.cpt
grdimage def_e.nc -R-100/100/-100/100 -JX3i -O -BagWSne -Ct.cpt -K -X3.5i >> $ps
grdtrack -Gdef_e.nc -ELM/RM > def_e.trk
psxy -R -J -O -K -W5p,white def_e.trk >> $ps
psxy -R -J -O -K -W1p,blue def_e.trk >> $ps
echo "-100 100 @~h@~" | pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 3. Compute the VGG anomaly
gravfft smt.nc+uk -D1670 -Nf -Fv -E$order -Gvgg.nc
# BR plot the VGG anomaly
makecpt -Crainbow -T-50/250/25 -Z > t.cpt
grdimage vgg.nc -R-100/100/-100/100 -JX3i -O -BagWsne -Ct.cpt -K -X-3.5i -Y3.25i >> $ps
grdtrack -Gvgg.nc -ELM/RM > vgg.trk
psxy -R -J -O -K -W5p,white vgg.trk >> $ps
psxy -R -J -O -K -W1p,red vgg.trk >> $ps
echo "-100 100 VGG" | pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 4. Compute the N-S deflection anomaly anomaly
gravfft smt.nc+uk -D1670 -Nf -Fn -E$order -Gdef_n.nc
# MR plot the N-S deflection anomaly
makecpt -Cpolar -T-120/120/20 -Z > t.cpt
grdimage def_n.nc -R-100/100/-100/100 -JX3i -O -BagWsne -Ct.cpt -K -X3.5i >> $ps
grdtrack -Gdef_n.nc -EBL/TR > def_n.trk
psxy -R -J -O -K -W5p,white def_n.trk >> $ps
psxy -R -J -O -K -W1p,orange def_n.trk >> $ps
echo "-100 100 @~x@~" | pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 5 Plot crossections of bathy and vgg crossections
# TL plot the bathy and vgg anomaly
psxy -R-100/100/-5100/1000 -JX3i/2.5i -O -K -W1p -i0,3 smt.trk -BafWsN -X-3.5i -Y3.2i >> $ps
echo "-100 1000 TOPO" | pstext -R -J -O -K -F+jTL+f12p -Dj0.1i/0.1i >> $ps
psxy -R-100/100/-50/250 -J -O -K -W1p,red -i0,3 vgg.trk -Bafg1000ENs >> $ps
echo "100 250 VGG" | pstext -R -J -O -K -F+jTR+f12p,Helvetica,red -Dj0.1i/0.1i >> $ps
# Add the two deflection crossections
# TRL plot the E-W and N-S deflection anomaly
psxy -R-100/100/-120/120 -JX3i/2.5i -O -K -W1p,blue -i0,3 def_e.trk -Bafg1000wsN -X3.5i >> $ps
echo "-100 120 @~h@~" | pstext -R -J -O -K -F+jTL+f12p,Helvetica,blue -Dj0.1i/0.1i >> $ps
psxy -R -J -O -K -W1p,orange -i1,3 def_n.trk -BafE >> $ps
echo "100 120 @~x@~" | pstext -R -J -O -K -F+jTR+f12p,Helvetica,orange -Dj0.1i/0.1i >> $ps
psxy -R -J -O -T >> $ps
