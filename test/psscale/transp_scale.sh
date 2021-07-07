#!/usr/bin/env bash
# Test CPTs with various levels of transparency
ps=transp_scale.ps
# 1. Discrete CPT with no transparenty
gmt makecpt -Cturbo -T-4000/-1000/500 -D > t.cpt
gmt psscale -Ct.cpt -R0/10/0/10 -JX15c -Y26c -Baf -Bx+l"Discrete CPT No Slices Transparent" -P -K > $ps
# 2. Discrete CPT with all transparent colors
gmt makecpt -A50 -Cturbo -T-4000/-1000/500 -D > t.cpt
gmt psscale -Ct.cpt -R -J -Y-5c -Baf -O -K -Bx+l"Discrete CPT All Slices Transparent" >> $ps
# 3. Continuous CPT with no transparency
gmt makecpt -Cturbo -T-4000/-1000 -D > t.cpt
gmt psscale -Ct.cpt -R -J -Y-5c -O -K -Baf -Bx+l"Continuous CPT No Slices Transparent" >> $ps
# 4. Continuous CPT with all transparent colors
gmt makecpt -A50 -Cturbo -T-4000/-1000 -D > t.cpt
gmt psscale -Ct.cpt -R -J -Y-5c -O -K -Baf -Bx+l"Continuous CPT All Slices Transparent" >> $ps
# 5. Discrete CPT with some transparent colors
gmt makecpt -Cturbo -T-4000/-1000/500 -D | awk '{if (NF == 5 && NR%2) {printf "%s\t%s@50\t%s\t%s@50\t%s\n", $1, $2, $3, $4, $5} else {print $0}}'> t.cpt
gmt psscale -Ct.cpt -R -J -Y-5c -O -Baf -Bx+l"Discrete CPT Odd Slices Transparent" >> $ps
