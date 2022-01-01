#!/usr/bin/env bash
# Make plot with variable line thickness
ps=varline.ps

# Relative course line file
gmt math -T0/90/2 -N5 T -C1-3 4 MUL SIND -C3 5 MUL 0.5 ADD ABS = line.txt
# Resample assuming dpi = 200
gmt psevents line.txt -Ar200 -R0/90/-1.2/1.2 -JX20c/10c --PROJ_LENGTH_UNIT=inch > line_pts.txt
# Make the plot
gmt makecpt -Cpolar -T-1/1 -D > my.cpt
gmt psevents line_pts.txt -R0/90/-1.2/1.2 -JX20c/10c -X2c -Y1.75c -Baf -Scp -Cmy.cpt -T60 -Es+d1 -Ms2+c1 -Mi0.5 -Mt+c0 > $ps
