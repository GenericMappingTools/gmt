#!/usr/bin/env bash
# Test lines between subplot with -W
gmt begin tictactoe ps
    gmt set FONT_ANNOT_PRIMARY 12p
    gmt subplot begin 3x3 -Fs5c+w1p,- -M0 -Blrbt
        gmt coast -Rg -JG30/30/?  -Gred -Bg -c0,0
        gmt coast -Rg -JG120/30/? -Gred -Bg -c1,1
        gmt coast -Rg -JG210/30/? -Gred -Bg -c2,2
    gmt subplot end
gmt end show
