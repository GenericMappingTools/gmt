#!/usr/bin/env bash
#
# Remove a bilinear robust trend
ps=trend_robust.ps
gmt grdtrend  -N3r -Ttrend.nc -Dresid.nc @data_w_nans.nc
gmt makecpt -T-5000/-4000 -Crainbow > t.cpt
gmt grdimage @data_w_nans.nc -JM3i -P -Baf -BWSne -Xc -Ct.cpt -K > $ps
gmt grdcontour trend.nc -J -Y3.25i -C5 -A10 -Baf -BWSne -O -K >> $ps
gmt makecpt -T-500/500 -Cjet > t.cpt
gmt grdimage resid.nc -J -Y3.25i -Ct.cpt -Baf -BWSne -O >> $ps
