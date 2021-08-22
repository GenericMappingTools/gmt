#!/usr/bin/env bash
# Demonstrate plotting of animated trajectories, part 2: Discretized line
# Create a line with a few points and get Cartesian distances to use as time,
# then sample densely to plot points to simulate the line
ps=trajectory2.ps
if [ $# -eq 0 ]; then   # Just make master PostScript frame 10
        opt="-M10,ps"
else    # Make MP4 at 2 frames per second
        opt="-Fmp4 -D2"
fi
cat << EOF | gmt mapproject -G+uc > trajectory.txt
0	0	
2	-0.6
3	0.2
6.5	2.7
8.5	-1
9.6	-2.1
EOF
cat << 'EOF' > pre.sh
# Create movie times (end distances is ~13.5 so go 0 to 14 over 24 frames)
gmt math -T0/14/24+n -o0 T = times.txt
# Make background static plot with entire dashed line and knots in black
gmt begin
	gmt events -R0/9.6/-2.7/2.7 -Jx0.5i -Ar${MOVIE_DPU} trajectory.txt --GMT_INTERPOLANT=linear > points.txt
	gmt plot trajectory.txt -W0.25p,- -B0 -X0 -Y0
	gmt plot -Sc9p -Gblack trajectory.txt
gmt end
EOF
# Make main script that plots trajectory
cat << 'EOF' > main.sh
gmt begin
	gmt events -R0/9.6/-2.7/2.7 -Jx0.5i -Sc3p -Gred points.txt -Es -T${MOVIE_COL0} -X0 -Y0
gmt end
EOF
# Build the product
gmt movie -C4.8ix2.7ix100 -Ntrajectory2 -Ttimes.txt -Sbpre.sh main.sh -Lf -M10 ${opt} -Z
