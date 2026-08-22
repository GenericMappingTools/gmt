#!/usr/bin/env bash
#
# Makes a plot of the six movie progress indicators available
#
# Make dummy map script for blank "movie"
ps=movie_indicator_de.ps
cat << EOF > map.sh
gmt begin
	gmt plot -R0/9.2/0/5 -Jx1i -T -X0.2i -Y0.2i -B0
gmt end
EOF
gmt movie map.sh -CHD -T1850/2010/5 -Fnone -M10,ps -Nmovie_indicator_de -Pe+w8i+ap+jTC+o0/0.3i -Pe+af+jTC+o0/0.8i -Pe+w5i+ae+s60+jTC+o0/1.3i -Pe+w4i+ac0+jTC+o0/1.8i -Pe+w3i+ac0+jTC+o0/2.3i  \
	-Pd+w2i+ac0+jTC+o0/2.8i -Pd+w3i+ac0+jTC+o0/3.3i -Pd+w5i+ae+s60+jTC+o0/3.8i -Pd+af+jTC+o0/4.3i -Pd+w8i+ap+jTC+o0/4.8i
