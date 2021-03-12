#!/usr/bin/env bash
#
# Makes a plot of progress indicator f in vertical setting with different lengths
#
# Make dummy map script for blank "movie"
ps=movie_indicator_f_ver.ps
cat << EOF > map.sh
gmt begin
	gmt plot -R0/9.2/0/5 -Jx1i -T -X0.2i -Y0.2i -B0
gmt end
EOF
gmt movie map.sh -CHD -T1850/2010/5 -Fnone -M10,ps -Nmovie_indicator_f_ver -Pf+ap+jML+o0.5i/0 -Pf+w4.5i+af+jML+o1i/0 -Pf+w4i+ae+s60+jML+o1.5i/0 -Pf+w4i+ac0+jML+o2i/0 -Pf+w2.5i+ac0+jML+o2.5i/0  \
	-Pf+w2i+ac0+jML+o3i/0 -Pf+w1.5i+ac0+jML+o3.5i/0 -Pf+ap+jMR+o0.5i/0 -Pf+w4.5i+af+jMR+o1i/0 -Pf+w4i+ae+s60+jMR+o1.5i/0 -Pf+w4i+ac0+jMR+o2i/0 -Pf+w2.5i+ac0+jMR+o2.5i/0  \
	-Pf+w2i+ac0+jMR+o3i/0 -Pf+w1.5i+ac0+jMR+o3.5i/0
