#!/usr/bin/env bash
#
# Makes a plot of the six movie progress indicators avalable
#
# Make dummy map script for "movie"
cat << EOF > map.sh
gmt begin
	gmt set MAP_VECTOR_SHAPE 0.75
	gmt psbasemap -R0/10/0/5 -JX7.6i/3.4i -Bafg -BWSrt+gbeige
gmt end
EOF
gmt movie map.sh -CHD -T50 -Fnone -M10,ps -NGMT_movie_progress -Pa+w1.8c+jTL -Pb+w1.8c+jTC+ap -Pc+w1.8c+jTR+ap -Pd+w7c+jLM+ap+f12p -Pe+w7c+ap+jRM+f12p -Pf+w20c+jBC+ap -W/tmp/junk -Z \
	-Ls"a)"+jTL+o2.2c/0.5c -Ls"b)"+jTL+o13.5c/0.5c -Ls"c)"+jTL+o21.5c/0.5c -Ls"d)"+jML+o0.5c/0  -Ls"e)"+jMR+o0.6c/0 -Ls"f)"+jBL+o1c/0.5c
