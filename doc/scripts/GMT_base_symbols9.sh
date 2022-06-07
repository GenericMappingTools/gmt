#!/usr/bin/env bash
#
# Plot psxy decorated line symbols for use on man page

gmt begin GMT_base_symbols9
    gmt set GMT_THEME cookbook
    gmt math -T0/360/1 T 5 MUL COSD 9 MUL T 180 SUB 120 DIV 2 POW NEG EXP MUL = t.txt
    # Just a cosine with line-following constant text a few times
    gmt plot -R0/360/-11/11 -JM6i -W1p,navy -S~n11:+ss0.1i+glightgreen+p0.1p,red t.txt -N
gmt end show
