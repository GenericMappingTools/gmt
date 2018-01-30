#!/bin/bash
#               GMT ANIMATION 06
#               $Id$
#
# Purpose:      Demonstrate aliasing by sampling a chirp
# GMT modules:  gmtmath, gmtconvert, psbasemap, psxy, pstext, movie, sample1d
# Unix progs:   echo, cat
# Note:         Run with any argument to build movie; otherwise 1200th frame is plotted only.

if [ $# -eq 1 ]; then   # Just make master PostScript frame 1200
        opt="-M1200,ps -Fnone"
	ps=anim_05.ps
else    # Make both movie formats and a thumbnail animated GIF using every 20th frame
        opt="-Fmp4 -Fwebm -A+l+s10"
fi      
# 0. Initial parameters
cat << EOF > init.sh
R=-R-7.5/2.5/-1.5/2	# Fixed plot domain window
J=-JX6.8i/4.2i		# Frame size after removing margin space
f=2			# Frequency of chirp at end time
EOF
# 1. Create background plot and data files needed in the loop
cat << EOF > pre.sh
gmt begin
	gmt psbasemap \$R \$J -X0.2i -Y0.3i -Bxa2.5g10 -By0g10 -B+gcornsilk --FORMAT_FLOAT_MAP=%+5.1f
	cat <<- END | gmt psxy -S0.3i -Ggreen
	0	-1.5	t
	0	+2.0	i
	END
	# Add Nyquist frequency in lower left corner
	echo "-7.5 -1.5 Nyquist frequency = 1 Hz" | gmt pstext -F+f18p,Helvetica-Bold+jBL -Dj0.1i/0.1i
gmt end
# Build chirp data sets; one ever 1 ms and one every 0.5 sec as samples
gmt math -T0/60/0.001 T 2 POW 2 DIV 60 DIV \$f MUL 2 MUL PI MUL COS = chirp.txt
gmt math -T0/1440/12 -Ca T 24 DIV -C1 2 POW 2 DIV 60 DIV \$f MUL 2 MUL PI MUL COS = chirp_samples.txt
gmt math -T0/1440/1 T 24 DIV = frame_times.txt
EOF
# 2. Set up the main frame script
cat << EOF > main.sh
gmt begin
	# Shift the chirp in time to simulate paper movement
	gmt math chirp.txt -C0 \${GMT_MOVIE_VAL2} SUB = chirp_shifted.txt
	# Plot the shifted chirp
	gmt psxy \$R \$J chirp_shifted.txt -W1p,red -X0.2i -Y0.3i
	# Compute index of most recent sample number
	last_sample=\`gmt math -Q \${GMT_MOVIE_FRAME} 12 DIV FLOOR RINT =\`
	# Extract all the old samples before the present
	gmt convert chirp_samples.txt -Z:\$last_sample > tmp.txt
	if [ -s tmp.txt ]; then
		gmt math tmp.txt -C0 \${GMT_MOVIE_VAL2} SUB = samples.txt
		gmt psxy -Sc0.3c -Gblue samples.txt
	fi
	# Take a new sample every 12 frames = 0.5 seconds
	take_sample=\`gmt math -Q \${GMT_MOVIE_FRAME} 12 MOD 0 EQ =\`
	if [ \${GMT_MOVIE_FRAME} -gt 12 ]; then	# Interpolating up to most recent sample
		gmt sample1d samples.txt -I0.001 > resampled.txt
		gmt psxy -W2.5p,blue resampled.txt
	fi
	if [ \$take_sample -eq 1 ]; then	# Take and plot sample at zero time
		y=\`gmt math -Q \${GMT_MOVIE_VAL2} 2 POW 2 DIV 60 DIV \$f MUL 2 MUL PI MUL COS =\`
		echo 0 \$y | gmt psxy -Sc0.5c -Gred
	fi
	# Add time counter in upper left corner
	printf "%4.1f 2 t = %6.3f s\n" -7.5 \${GMT_MOVIE_VAL2} | gmt pstext -F+f18p,Helvetica-Bold+jTL -Dj0.1i/0.1i
	# Add cycles counter in upper right corner
	fnow=\`gmt math -Q \${GMT_MOVIE_VAL2} 60 DIV \$f MUL =\`
	printf "2.5 2 f = %6.4f Hz\n" \$fnow | gmt pstext -F+f16p,Helvetica-Bold+jTR -Dj0.1i/0.1i
	# Add frame counter in lower right corner
	printf "2.5 -1.5 %04d\n" \${GMT_MOVIE_FRAME} | gmt pstext -F+f14p,Helvetica-Bold+jBR -Dj0.1i/0.1i
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Sbpre.sh -C7.2ix4.8ix100 -Iinit.sh -Tframe_times.txt -Nanim_06 -Z $opt
rm -rf init.sh main.sh pre.sh
