#!/usr/bin/env bash
#
# Test the output of gmt grdflexure for single circular Gaussian seamount
# on a general linear viscoelastic foundation
# DVC_TEST

ps=flexure_gl.ps
m=g
f=0.2
gmt set MAP_FRAME_TYPE plain GMT_FFT kiss
cat << EOF > t.txt
#lon lat azimuth, semi-major, semi-minor, height
300	200	0	50	50	5000
EOF
gmt grdseamount -R0/600/0/400+uk -I1000 -Gsmt.nc t.txt -Dk -E -F$f -C$m
gmt grdcontour smt.nc+Uk -Jx0.01i -Xc -P -A1 -GlLM/RM -Bafg -K -Z+s0.001 > $ps
gmt grdflexure smt.nc -D3300/2700/2400/1030 -E20k/10k -M10k -Gflex_gl_%g.nc -T50k
gmt grdcontour flex_gl_50000.nc+Uk -J -O -C0.2 -A1 -Z+s0.001 -GlLM/RM -Bafg -BWsNE+t"GLVE Plate Flexure, T@-e@- = 20/10 km, t/t@-m@- = 5" -Y4.4i >> $ps
