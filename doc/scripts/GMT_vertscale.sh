#!/usr/bin/env bash
gmt begin GMT_vertscale
	gmt math -T-8/6/0.01 -N3/0 -C2 T 3 DIV 2 POW NEG EXP T PI 2 MUL MUL COS MUL 50 MUL = t.txt
	gmt wiggle -R-10/10/-3/3 -JM6i -B -Z100i -DjRM+w100+lnT t.txt -Tfaint -W1p -BWSne --MAP_FRAME_TYPE=plain
gmt end show
