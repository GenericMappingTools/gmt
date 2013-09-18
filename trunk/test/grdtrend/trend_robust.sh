#!/bin/bash
#	$Id$
#
# Remove a bilinear robust trend
ps=trend_robust.ps
grdtrend  -N3r -Ttrend.nc -Dresid.nc data_w_nans.nc
makecpt -T-5000/-4000/100 -Crainbow -Z > t.cpt
grdimage data_w_nans.nc -JM3i -P -Baf -BWSne -Xc -Ct.cpt -K > $ps
grdcontour trend.nc -J -Y3.25i -C5 -A10 -Baf -BWSne -O -K >> $ps
makecpt -T-500/500/100 -Cjet -Z > t.cpt
grdimage resid.nc -J -Y3.25i -Ct.cpt -Baf -BWSne -O >> $ps
