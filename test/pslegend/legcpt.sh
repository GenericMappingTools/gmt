#!/bin/bash
# Several failures:
# Using frame -B and CPT -B in pslegend fails
# 1. filling of frame (lightgray) is shifted
# 2. filling of frame is delayed and plots on top of legend
# 3. The -B annotation/frametick settings for the CPT is ignored
#    since a -B is parsed twice.
ps=legcpt.ps
gmt makecpt -Cabyss > col.cpt
cat << EOF > leg
H 16 1 10 events during Monday to Friday
D 0 1p
B col.cpt 1c 1c -Ba1000f100g500+lm
G 0.7c
D 0 1p
G 0.5c
G 0.5c
EOF
gmt psbasemap -JX15/10c -R0/25/0/3 -P -Bafg -B+glightgray+t"Only psbasemap sets frame -B" -K -Xc > $ps
gmt pslegend leg -Dn0.5/0.5+w12c/4.5c+jCM -J -R -P -O -K -F+p+glightyellow >> $ps
gmt pslegend leg -Dn0.5/0.5+w12c/4.5c+jCM -J -R -O -F+p+glightyellow -Bafg -B+glightgray+t"pslegend sets frame/CPT -B (CPT ignored)" -Y13c >> $ps
gmt psconvert -Tf -P $ps
