#!/usr/bin/env bash
# Testing gmt spatial spherical area and centroid

cat << EOF > pol.txt
-45	0
45	0
45	45
-45	45
-45	30
0	30
0	15
-45	15
EOF
gmt begin sphareacentroid ps
  gmt subplot begin 2x1 -Fs12c -Bafg -Rg -M0 -Y1.25c
    # N hemisphere
    cp pol.txt P.txt
    gmt vector P.txt -Am -fg -E > mean.txt
    echo "-45	0" >> P.txt
    N=$(gmt convert -i0:1 mean.txt --IO_COL_SEPARATOR=/)
    gmt plot P.txt -JG0/10/? -W2p -c
    gmt plot P.txt -SE-300 -Gblack
    gmt plot P.txt -W0.5p,blue -Fr${N}
    gmt plot -SE-300 -Gred mean.txt
    gmt spatial P.txt -Q -fg > tmp
    gmt plot tmp -Sa0.5c -Gyellow -W0.25p
    gmt text tmp -F+jLM+z%.0lf -Dj0.5c
    # S hemisphere
    gmt math -Ca pol.txt -C1 -1 MUL = P.txt
    gmt vector P.txt -Am -fg -E > mean.txt
    echo "-45	0" >> P.txt
    N=$(gmt convert -i0:1 mean.txt --IO_COL_SEPARATOR=/)
    gmt plot P.txt -JG0/-10/? -W2p -c
    gmt plot P.txt -SE-300 -Gblack
    gmt plot P.txt -W0.5p,blue -Fr${N}
    gmt plot -SE-300 -Gred mean.txt
    gmt spatial P.txt -Q -fg > tmp
    gmt plot tmp -Sa0.5c -Gyellow -W0.25p
    gmt text tmp -F+jLM+z%.0lf -Dj0.5c
  gmt subplot end
gmt end show
