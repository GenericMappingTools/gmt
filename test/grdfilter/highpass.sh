#!/usr/bin/env bash
# Test highpass-filter with or without coarse grid low-pass option
# Doing a 10x10 degree patch near Nigeria
# DVC_TEST
ps=highpass.ps
gmt grdcut @earth_relief_02m -R0/10/0/10 -GAFR.nc
gmt grdfilter AFR.nc -Fg100+h -D2 -GHc.nc -I15m
gmt grdfilter AFR.nc -Fg100 -D2 -GLc.nc -I15m
gmt grdfilter AFR.nc -Fg100+h -D2 -GHf.nc
gmt grdfilter AFR.nc -Fg100 -D2 -GLf.nc
gmt grdimage AFR.nc -JM3i -I -P -K -Baf -BWSne -Y0.75i -Cturbo > $ps
gmt grdimage Lc.nc -J -I -O -K -Baf -BWsne -Y3.25i >> $ps
gmt grdimage Hc.nc -J -I -O -K -Baf -BWsne -Y3.25i >> $ps
gmt grdimage AFR.nc -J -I -O -K -Baf -BWSnE -X3.5i -Y-6.5i -Cturbo >> $ps
gmt grdimage Lf.nc -J -I -O -K -Baf -BWsnE -Y3.25i >> $ps
gmt grdimage Hf.nc -J -I -O -Baf -BWsnE -Y3.25i >> $ps
