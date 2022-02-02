#!/usr/bin/env bash
# Test -Ts with a categorical CPT
# DVC_TEST
ps=tiles.ps
echo "0 P35+r200" > pattern.cpt
echo "1 p5+r200" >> pattern.cpt
gmt grdmath -R-3/3/-3/3 -r -I1 0 0 CDIST 1 GT = test.nc
gmt grdview test.nc -JX6i -T+s -Cpattern.cpt -B0 -P > $ps
