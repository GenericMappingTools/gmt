#!/usr/bin/env bash
# Test plot3d -N with lines and polygons in 3-D
cat << EOF > z.txt
-1	0	0
2	1	0
3	-1	0
1	-1.5	0
EOF

gmt begin noclip_lines3 ps
	gmt subplot begin 3x2 -Fs6c -R0/3/-2/2 -M32p/4p -Y6c
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -W2p -c
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -W2p -N -c
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -Glightgray -W1p -c
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -Glightgray -W1p -N -c
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -Glightgray -c
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -Glightgray -W1p -N -c
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -W1p,- -N -L
	gmt plot3d -R0/3/-2/2 -p155/35 z.txt -W3p -L
	gmt subplot end
gmt end
