#!/usr/bin/env bash
# Test setting of pen width and color via cpt and headers

cat << EOF > t.txt
> -W0.25p -Z1
6.5 1.5 0
6.5 0.5 0
> -W6p -Z2
7.5 1.5 0
7.5 0.5 0
EOF
gmt begin varpen ps
	gmt makecpt -Cjet -T0/3
	gmt plot3d t.txt -R6/8/0/2 -JX12c -W+cl -B -BWSrt -C -p135/35
gmt end show
