#!/usr/bin/env bash
gmt math -T10/30/1 T 20 SUB 10 DIV 2 POW 41.5 ADD = line.txt

gmt begin GMT_lineoffset
	gmt plot line.txt -R8/32/40/44 -JM5i -Wfaint,red -Bxaf -Bya2f1 -BWSne --MAP_FRAME_TYPE=plain
	gmt plot line.txt -W2p+o1c/500k
	gmt text -F+f10p+jCM+a << EOF
11.0 42.6 -11.5 1 cm
27.1 42.3 +9.5 500 km
EOF
gmt end show
