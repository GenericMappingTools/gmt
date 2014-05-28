#!/bin/bash
#	$Id$
#
# Compute E-W and N-S deflections over synthetic seamount
ps=deflections.ps
order=2
# 1. Create a bathymetry data set with one circular truncated seamount
#    as in Fig 3. of Marks & Smith, 2007 [GRL], with R_base = 35 km,
# R_top = 10 km, height = 3751 m, depth = -5084 m, density d_rho = 2800-1030
# = 1670 kg/m^3, so the flattening is 10/25 = 0.4.
echo "0	0	25	3751" | gmt grdseamount -R-256/256/-256/256 -I1 -r -C -Gsmt.nc -F0.4 -Z-5084
# BL Plot the bathymetry
gmt makecpt -Crainbow -T-5100/-1000/200 -Z > t.cpt
gmt grdimage smt.nc -R-100/100/-100/100 -JX3i -P -Bag -BWSne -Ct.cpt -K > $ps
gmt grdtrack -Gsmt.nc -ELM/RM+d > smt.trk
gmt psxy -R -J -O -K -W5p,white smt.trk >> $ps
gmt psxy -R -J -O -K -W1p smt.trk >> $ps
echo "-100 100 BATHYMETRY" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 2. Compute the E-W deflection anomaly
gmt gravfft smt.nc+uk -D1670 -Nf+a -Fe -E$order -Gdef_e.nc
# ML plot the E-W deflection anomaly
gmt makecpt -Cpolar -T-120/120/20 -Z > t.cpt
gmt grdimage def_e.nc -R-100/100/-100/100 -JX3i -O -Bag -BWSne -Ct.cpt -K -X3.5i >> $ps
gmt grdtrack -Gdef_e.nc -ELM/RM+d > def_e.trk
gmt psxy -R -J -O -K -W5p,white def_e.trk >> $ps
gmt psxy -R -J -O -K -W1p,blue def_e.trk >> $ps
echo "-100 100 @~h@~" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 3. Compute the VGG anomaly
gmt gravfft smt.nc+uk -D1670 -Nf+a -Fv -E$order -Gvgg.nc
# BR plot the VGG anomaly
gmt makecpt -Crainbow -T-50/250/25 -Z > t.cpt
gmt grdimage vgg.nc -R-100/100/-100/100 -JX3i -O -Bag -BWsne -Ct.cpt -K -X-3.5i -Y3.25i >> $ps
gmt grdtrack -Gvgg.nc -ELM/RM+d > vgg.trk
gmt psxy -R -J -O -K -W5p,white vgg.trk >> $ps
gmt psxy -R -J -O -K -W1p,red vgg.trk >> $ps
echo "-100 100 VGG" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 4. Compute the N-S deflection anomaly anomaly
gmt gravfft smt.nc+uk -D1670 -Nf+a -Fn -E$order -Gdef_n.nc
# MR plot the N-S deflection anomaly
gmt makecpt -Cpolar -T-120/120/20 -Z > t.cpt
gmt grdimage def_n.nc -R-100/100/-100/100 -JX3i -O -Bag -BWsne -Ct.cpt -K -X3.5i >> $ps
gmt grdtrack -Gdef_n.nc -EBL/TR+d > def_n.trk
gmt psxy -R -J -O -K -W5p,white def_n.trk >> $ps
gmt psxy -R -J -O -K -W1p,orange def_n.trk >> $ps
echo "-100 100 @~x@~" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i/0.1i -Gwhite -TO >> $ps
# 5 Plot crossections of bathy and vgg crossections
# TL plot the bathy and vgg anomaly
gmt psxy -R-100/100/-5100/1000 -JX3i/2.5i -O -K -W1p -i0,3 smt.trk -Baf -BWsN -X-3.5i -Y3.2i >> $ps
echo "-100 1000 TOPO" | gmt pstext -R -J -O -K -F+jTL+f12p -Dj0.1i/0.1i >> $ps
gmt psxy -R-100/100/-50/250 -J -O -K -W1p,red -i0,3 vgg.trk -Bafg1000 -BENs >> $ps
echo "100 250 VGG" | gmt pstext -R -J -O -K -F+jTR+f12p,Helvetica,red -Dj0.1i/0.1i >> $ps
# Add the two deflection crossections
# TRL plot the E-W and N-S deflection anomaly
gmt psxy -R-100/100/-120/120 -JX3i/2.5i -O -K -W1p,blue -i0,3 def_e.trk -Bafg1000 -BwsN -X3.5i >> $ps
echo "-100 120 @~h@~" | gmt pstext -R -J -O -K -F+jTL+f12p,Helvetica,blue -Dj0.1i/0.1i >> $ps
gmt psxy -R -J -O -K -W1p,orange -i1,3 def_n.trk -Baf -BE >> $ps
echo "100 120 @~x@~" | gmt pstext -R -J -O -K -F+jTR+f12p,Helvetica,orange -Dj0.1i/0.1i >> $ps
gmt psxy -R -J -O -T >> $ps
