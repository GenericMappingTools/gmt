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
gmt movie map.sh -CHD -T50 -Fnone -M10,ps -NGMT_movie_progress -Pa+jTL -Pb+jTC+ap -Pc+ap -Pd+jLM+ap+f12p -Pe+ap+jRM+f12p -Pf+ap -W/tmp/junk -Z \
	-Ls"a)"+jTL+o1.7c/0.5c -Ls"b)"+jTL+o13.3c/0.5c -Ls"c)"+jTL+o22c/0.5c -Ls"d)"+jML+o0.6c/0  -Ls"e)"+jMR+o0.6c/0 -Ls"f)"+jBL+o3.8c/0.5c
