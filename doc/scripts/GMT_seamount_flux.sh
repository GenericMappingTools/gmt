#!/usr/bin/env bash
# Illustrate the two different volume-flux curves
ps=GMT_seamount_flux.ps
gmt set FONT_ANNOT_PRIMARY 14p
# Two flux curves
gmt math -T0/1/1 T = | gmt psxy -R0/1/0/1.1 -JX6.5i/1i -W1p,- -Bxa0.25fg0.5 -Byafg0.5 -Bx+l"Normalized seamount lifespan (@%6%t/@~D@~@%6%t@%%)" -By+l"@%6%V(t)/V@-0@-@%%" -BWSne -K --FONT_LABEL=16p,Helvetica,black -P > $ps
gmt math -T0/1/0.01 T 0.5 SUB 6 MUL ERF 2 DIV 0.5 ADD = | gmt psxy -R -J -O -W3p >> $ps
