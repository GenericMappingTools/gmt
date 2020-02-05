#!/usr/bin/env bash
#
# Makes a plot of progress indicator f in horizontal setting with different lengths
#
# Make dummy map script for blank "movie"
ps=movie_indicator_f_hor.ps
cat << EOF > map.sh
gmt begin
	gmt plot -R0/9.2/0/5 -Jx1i -T -X0.2i -Y0.2i -B0
gmt end
EOF
gmt movie map.sh -CHD -T1850/2010/5 -Fnone -M10,ps -Nmovie_indicator_f_hor -Pf+ap+jTC+o0/0.2i -Pf+w8i+af+jTC+o0/0.7i -Pf+w7i+ae+s60+jTC+o0/1.2i -Pf+w6i+ac0+jTC+o0/1.7i -Pf+w5i+ac0+jTC+o0/2.2i  \
	-Pf+w4i+ac0+jTC+o0/2.7i -Pf+w3i+ac0+jTC+o0/3.2i -Pf+w2.5i+ae+s60+jTC+o0/3.7i -Pf+w2i+af+jTC+o0/4.2i -Pf+w1.5i+ap+jTC+o0/4.7i
