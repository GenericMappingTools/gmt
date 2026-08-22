#!/usr/bin/env bash
# Make sure cycle symbol is not overprinting labels or nan box (issue #1175)
ps=cyclecpt.ps
gmt makecpt -Ccyclic -T0/180 -Ww > temp_cpt.cpt
# Vertical
gmt psscale -P -K -Ctemp_cpt.cpt -Dx3c/5c+w10c/0.618c+n -Bxa+l"Cyclic CPT with NaN and unit" -By+l"m" -X1i > $ps
gmt psscale -O -K -Ctemp_cpt.cpt -Dx3c/5c+w10c/0.618c+n -Bxa+l"Cyclic CPT with NaN" -X4i >> $ps
# Horizontal
gmt psscale -O -K -Ctemp_cpt.cpt -Dx5c/5c+w10c/0.618c+n+h -Bxa+l"Cyclic CPT with NaN and unit" -By+l"m" -X-4i -Y5i >> $ps
gmt psscale -O -Ctemp_cpt.cpt -Dx5c/5c+w10c/0.618c+n+h -Bxa+l"Cyclic CPT with NaN" -Y1i >> $ps
