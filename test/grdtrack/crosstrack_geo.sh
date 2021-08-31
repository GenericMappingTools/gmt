#!/usr/bin/env bash
# Test non-orthogonal modifier +d in -C for grdtrack, geographic version
# Also try +l for left-side only
gmt begin crosstrack_geo ps
	# Fake geographic grid
	gmt set PS_MEDIA letter
	gmt grdmath -R-10/10/-5/5 -I0.1 -fg X Y MUL = geo.grd
	gmt subplot begin 3x1 -Fs14c/7c -Sc -Srl -A -T"Geographic cross-tracks"
		gmt subplot set 0 -A"No deviation"
		# Sample it along profiles orthogonal to the line y = 0
		gmt grdtrack -Ggeo.grd -C800k/50k/100k -E-7/0/7/0 -Dline.txt > cross.txt
		# Plot it
		echo -9 0 9 0 | gmt plot -Jm? -Sv12p+s+e+p0.5p -Gred -W0.5p,-
		gmt plot line.txt -W3p
		gmt plot cross.txt -W1p
		gmt subplot set 1 -A"+30 deviation, left-side only"
		# Sample it along profiles with 30 degree deviation to the line y = 0
		gmt grdtrack -Ggeo.grd -C800k/50k/100k+d30+l -E-7/0/7/0 -Dline.txt > cross.txt
		# Plot it
		echo -9 0 9 0 | gmt plot -Jm? -Sv12p+s+e+p0.5p -Gred -W0.5p,-
		gmt plot line.txt -W3p
		gmt plot cross.txt -W1p
		# Sample it along profiles with -30 degree deviation to the line y = 0
		gmt subplot set 2 -A"-30 deviation"
		gmt grdtrack -Ggeo.grd -C800k/50k/100k+d-30 -E-7/0/7/0 -Dline.txt > cross.txt
		# Plot it
		echo -9 0 9 0 | gmt plot -Jm? -Sv12p+s+e+p0.5p -Gred -W0.5p,-
		gmt plot line.txt -W3p
		gmt plot cross.txt -W1p
	gmt subplot end
gmt end show
