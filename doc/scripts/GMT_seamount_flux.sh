#!/usr/bin/env bash
# Illustrate the two different volume-flux curves
gmt begin GMT_seamount_flux
    gmt set GMT_THEME cookbook
    gmt set FONT_ANNOT_PRIMARY 14p
    # Two flux curves
    gmt math -T0/1/1 T = | gmt plot -R0/1/0/1.1 -JX6.5i/1i -W1p,- -Bxa0.25fg0.5 -Byafg0.5 -Bx+l"Normalized seamount lifespan (@%6%t/@~D@~@%6%t@%%)" -By+l"@%6%V(t)/V@-0@-@%%" -BWSne --FONT_LABEL=16p,Helvetica,black
    gmt math -T0/1/0.01 T 0.5 SUB 3 MUL ERF 2 SQRT DIV 0.5 ADD NORM = | gmt plot -W3p
gmt end show
