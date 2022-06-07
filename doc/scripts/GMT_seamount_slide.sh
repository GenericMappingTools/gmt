#!/usr/bin/env bash
# Illustrate a Polynomial seamount with two slides
gmt begin GMT_seamount_slide
    gmt set GMT_THEME cookbook
    echo 0 0 30 5 | gmt grdseamount -R-45/40/-45/32/0/20 -I0.05 -Gsmt.grd -Co -ZNaN -F0.17 -S+h1/4+d1+a25/170+u0.2 -S+h0.7/4.7+d0.7+a200/260+u0.1+p8
    gmt grdview smt.grd -Qi -I+a65+ne0.8 -Jx0.2c -JZ3c -p150/20 -Cbatlow
gmt end show
