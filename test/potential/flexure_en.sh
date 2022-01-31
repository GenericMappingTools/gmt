#!/usr/bin/env bash
#
# Test the output of gmt grdflexure for single circular Gaussian seamount
# on elastic plate with in-plane stresses
# DVC_TEST

ps=flexure_en.ps
m=g
f=0.2
gmt set MAP_FRAME_TYPE plain GMT_FFT kiss
cat << EOF > t.txt
#lon lat azimuth, semi-major, semi-minor, height
300	200	0	50	50	5000
EOF
gmt grdseamount -R0/600/0/400+uk -I1000 -Gsmt.nc t.txt -Dk -E -F$f -C$m
gmt grdcontour smt.nc+Uk -Jx0.01i -Xc -P -A1 -GlLM/RM -Bafg -K -Z+s0.001 > $ps
gmt grdflexure smt.nc -D3300/2700/2400/1030 -E5k -A-4e11/2e11/-1e12 -Gflex_e.nc
gmt grdcontour flex_e.nc+Uk -J -O -C0.2 -A1 -Z+s0.001 -GlLM/RM -Bafg -BWsNE+t"Elastic Plate Flexure, T@-e@- = 5 km, N = (-4/2/-10)e12 Pa" -Y4.4i >> $ps
