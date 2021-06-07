#!/usr/bin/env bash
#
# We demonstrate aliasing by sampling a linear chirp signal and then try to reconstruct
# the original signal using a cubic spline interpolator through the samples.  Ideally, we
# should do this via the Shannon-Whittaker sinc function but alas not in GMT yet.  As the
# frequency of the chirp increases we find it harder and harder to reconstruct a reasonable
# representation of the original signal from the samples.  The morale is you need to sample
# data as often as you are able to.  Here, we added a title slide visible for 6 seconds, then
# fade out to the animation.  The scripts are a bit longer due to lots of little details.
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/3vB53hoLsls
# The movie took ~3 minutes to render on a 24-core MacPro 2013.

rate=24			# Frames per seconds
frames=$(gmt math -Q 60 $rate MUL =)
# 0. Initial parameters
cat << EOF > init.sh
R=-R-7.5/2.5/-1.5/2	# Fixed plot domain window
J=-JX22c/11.5c		# Frame size after removing margin space
f=2			# Frequency of chirp in Hz at end time
rate=$rate
frames=$frames
EOF
# 1. Make a title slide explaining things
cat << 'EOF' > title.sh
gmt begin
	echo "12 11.5 Demonstration of aliasing by sampling a linear chirp" | gmt text -R0/24/0/13.5 -Jx1c -F+f26p,Helvetica-Bold+jCB -X0 -Y0
	echo "12 10.5 y(t) = cos (2@~p@~t@+2@+/60)" | gmt text -F+f36p,Times-Italic+jTC
	gmt text -M -F+f14p <<- END
	> 12 6.5 16p 20c j
	We will simulate sampling the continuous phenomenon described by @%6%y(t)@%% every 0.5 seconds,
	meaning our Nyquist frequency is 1 Hz.  The samples are then interpolated with a cubic spline to
	reconstruct the original signal@+*@+.  At first, this works great, but as the chirp increases
	in frequency we find the interpolation starts to deviate from the actual phenomenon, and eventually
	the reconstruction is clearly dominated by aliased frequencies much longer than those in the phenomenon.
	END
	gmt legend -Dx12c/3.5c+w20c+jTC+l1.2 -C0.3c -F+p+gazure1 <<- END
	N 3
	V 0 1p
	S 1.5c - 2.5c - 1p,red 3.2c Chirp (pheonomenon)
	S 1.5c c 0.3c blue - 2c Sampled values
	S 0.5c - 2.5c - 2.5p,blue 2.2c Spline interpolator
	END
	echo "@+*@+Sorry, we do not have a Shannon-Whittaker @%6%sinc@%% interpolator in GMT (yet)" | gmt text -F+f10p+cBR -Dj0.5c
gmt end show
EOF
# 2. Create background plot and build data files needed in the main script
cat << 'EOF' > pre.sh
gmt begin
	gmt plot $R $J -X1c -Y1c -B+gcornsilk -S1c -Ggreen <<- END
	0	-1.5	t
	0	+2.0	i
	END
	gmt basemap -Bxa2.5g10 -By0g10  --FORMAT_FLOAT_MAP=%+5.1f
	# Display Nyquist frequency in lower left corner
	echo "Nyquist frequency = 1 Hz" | gmt text -F+f18p,Helvetica-Bold+jBL+cBL -Dj0.3c
gmt end
# Build chirp data sets for 1 minute (60 secs); one every 1 ms and one every 0.5 sec as samples
gmt math -T0/60/0.001 T 2 POW 2 DIV 60 DIV $f MUL 2 MUL PI MUL COS = chirp.txt
gmt math -T0/60/0.5 -Ca T -C1 2 POW 2 DIV 60 DIV $f MUL 2 MUL PI MUL COS = chirp_samples.txt
gmt math -T0/$frames/1 T $rate DIV = frame_times.txt
EOF
# 3. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
	# Shift the chirp in time to simulate paper movement
	gmt math chirp.txt -C0 ${MOVIE_COL1} SUB = chirp_shifted.txt
	# Plot the shifted chirp
	gmt plot $R $J chirp_shifted.txt -W1p,red -X1c -Y1c
	# Compute index of most recent sample number
	last_sample=$(gmt math -Q ${MOVIE_FRAME} $rate 2 DIV DIV FLOOR RINT =)
	# Extract all the old samples before the present
	gmt convert chirp_samples.txt -Z:$last_sample > tmp.txt
	if [ -s tmp.txt ]; then
		gmt math tmp.txt -C0 ${MOVIE_COL1} SUB = samples.txt
		gmt plot -Sc0.3c -Gblue samples.txt
	fi
	# Take a new sample every 12 frames = 0.5 seconds
	take_sample=$(gmt math -Q ${MOVIE_FRAME} $rate 2 DIV MOD 0 EQ =)
	if [ ${MOVIE_FRAME} -gt 12 ]; then	# Interpolating up to most recent sample
		gmt sample1d samples.txt -I0.001 > resampled.txt
		gmt plot -W2.5p,blue resampled.txt
	fi
	if [ $take_sample -eq 1 ]; then	# Take and plot sample at zero time
		y=$(gmt math -Q ${MOVIE_COL1} 2 POW 2 DIV 60 DIV $f MUL 2 MUL PI MUL COS =)
		echo 0 $y | gmt plot -Sc0.5c -Gred
	fi
	# Add time counter in upper left corner
	printf "t = %6.3f s\n" ${MOVIE_COL1} | gmt text -F+f18p,Helvetica-Bold+jTL+cTL -Dj0.3c
	# Add cycles counter in upper right corner
	fnow=$(gmt math -Q ${MOVIE_COL1} 60 DIV $f MUL =)
	printf "f = %6.4f Hz\n" $fnow | gmt text -F+f16p,Helvetica-Bold+jTR+cTR -Dj0.3c
	# Add frame counter in lower right corner
	printf "%04d\n" ${MOVIE_FRAME} | gmt text -F+f14p,Helvetica-Bold+jBR+cBR -Dj0.3c
gmt end
EOF
# 4. Run the movie
gmt movie main.sh -Sbpre.sh -CHD -Iinit.sh -Tframe_times.txt -D$rate -Etitle.sh+d6s+fo1s+gwhite -Nanim06 -H8 -Zs -Fmp4 -W -V
