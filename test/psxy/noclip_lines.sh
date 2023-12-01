#!/usr/bin/env bash
# Test plot -N with lines and polygons in 2-D
cat << EOF > t.txt
-1	0
2	1
3	-1
1	-1.5
EOF

gmt begin noclip_lines ps
	gmt subplot begin 3x2 -Fs6c -R0/3/-2/2 -M32p/4p -Y6c
	gmt plot t.txt -W2p -c
	gmt plot t.txt -W2p -N -c
	gmt plot t.txt -Glightgray -W1p -c
	gmt plot t.txt -Glightgray -W1p -N -c
	gmt plot t.txt -Glightgray -c
	gmt plot t.txt -Glightgray -W1p -N -c
	gmt plot t.txt -W1p,- -N -L
	gmt plot t.txt -W3p -L
	gmt subplot end
gmt end
