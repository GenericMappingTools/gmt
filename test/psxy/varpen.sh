#!/usr/bin/env bash
# Test setting of pen width and color via cpt and headers

cat << EOF > t.txt
> -W0.5p -Z1
6.5 1.5
6.5 0.5
> -W6p -Z2
7.5 1.5
7.5 0.5
EOF
gmt begin varpen ps
	gmt makecpt -Cjet -T0/3
	gmt plot t.txt -R6/8/0/2 -JX15c -W+cl -B -BWSrt -C
gmt end show
