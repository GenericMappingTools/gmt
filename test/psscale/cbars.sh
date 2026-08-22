#!/usr/bin/env bash
ps=cbars.ps
# Use CPT's low/high colors
gmt makecpt -Ccool -Ic -T-6000/0 -N > 1.cpt
gmt makecpt -Cdem2 -D -T0/5000 >> 1.cpt
# Use current GMT default fore/back colors
gmt makecpt -Ccool -Ic -T-6000/0 -N > 2.cpt
gmt makecpt -Cdem2 -M -T0/5000 >> 2.cpt
# Use defaults at the time of run
gmt makecpt -Ccool -Ic -T-6000/0 -N > 3.cpt
gmt makecpt -Cdem2 -N -T0/5000 >> 3.cpt
# Plot the three scales
gmt psscale -Baf -Dx1i/1i+w7i+v+e -P -K -C1.cpt  > $ps
gmt psscale -Baf -Dx3i/1i+w7i+v+e -O -K -C2.cpt >> $ps
gmt psscale -Baf -Dx5i/1i+w7i+v+e -O -C3.cpt --COLOR_BACKGROUND=blue --COLOR_FOREGROUND=red >> $ps
