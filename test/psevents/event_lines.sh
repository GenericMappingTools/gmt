#!/usr/bin/env bash
# Test the 5 possible time curves for each interpolator

gmt psevents -/ll
mv -f psevents_function.txt psevents_function_l.txt
gmt psevents -/qq
mv -f psevents_function.txt psevents_function_q.txt
gmt psevents -/cc
mv -f psevents_function.txt psevents_function_c.txt
cat << EOF > /tmp/labels
-0.5	2.4	RISE
0.5	2.4	PLATEAU
1.5	2.4	DECAY
2.5	2.4	NORMAL
3.5	2.4	FADE
EOF
cat << EOF > /tmp/def
-2	0
0	0
0	1
3	1
3	0
5	0
EOF
gmt begin event_lines ps
	gmt subplot begin 3x1 -Fs15c/5c -R-2/5/-1/2.5 -Bafg1 -A+jTL+gwhite+p1p
	gmt subplot set 0 -ALinear
	gmt plot psevents_function_l.txt -W3p,red -i0,1 -lsize
	gmt plot psevents_function_l.txt -W2p,green -i0,2 -lintens
	gmt plot psevents_function_l.txt -W1p,blue  -i0,3+d100 -ltransp
	gmt plot psevents_function_l.txt -W0.25p,orange -i0,4 -ldz
	gmt text /tmp/labels -F+f9p+jTC
	gmt plot /tmp/def -W1p,3_2:0 -ldefault
	gmt subplot set 1 -AQuadratic
	gmt plot psevents_function_q.txt -W3p,red -i0,1 -lsize
	gmt plot psevents_function_q.txt -W2p,green -i0,2 -lintens
	gmt plot psevents_function_q.txt -W1p,blue  -i0,3+d100 -ltransp
	gmt plot psevents_function_q.txt -W0.25p,orange -i0,4 -ldz
	gmt text /tmp/labels -F+f9p+jTC
	gmt plot /tmp/def -W1p,3_2:0 -ldefault
	gmt subplot set 2 -ACosine
	gmt plot psevents_function_c.txt -W3p,red -i0,1 -lsize
	gmt plot psevents_function_c.txt -W2p,green -i0,2 -lintens
	gmt plot psevents_function_c.txt -W1p,blue  -i0,3+d100 -ltransp
	gmt plot psevents_function_c.txt -W0.25p,orange -i0,4 -ldz
	gmt text /tmp/labels -F+f9p+jTC
	gmt plot /tmp/def -W1p,3_2:0 -ldefault
	gmt subplot end
gmt end show
