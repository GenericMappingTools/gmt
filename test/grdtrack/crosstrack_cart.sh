#!/usr/bin/env bash
# Test non-orthogonal modifier +d in -C for grdtrack, Cartesian version
# Also try +r for right-side only
gmt begin crosstrack_cart ps
	# Fake Cartesian grid
	gmt set PS_MEDIA letter
	gmt grdmath -R-10/10/-5/5 -I0.1 X Y MUL = cart.grd
	gmt subplot begin 3x1 -Fs14c/7c -Sc -Srl -A -T"Cartesian cross-tracks"
		gmt subplot set 0 -A"No deviation"
		# Sample it along profiles orthogonal to the line y = 0
		gmt grdtrack -Gcart.grd -C8/0.1/1 -E-7/0/7/0 -Dline.txt > cross.txt
		# Plot it
		echo -9 0 9 0 | gmt plot -Jx? -Sv12p+s+e+p0.5p -Gred -W0.5p,-
		gmt plot line.txt -W3p
		gmt plot cross.txt -W1p
		gmt subplot set 1 -A"+30 deviation"
		# Sample it along profiles with 30 degree deviation to the line y = 0
		gmt grdtrack -Gcart.grd -C8/0.1/1+d30 -E-7/0/7/0 -Dline.txt > cross.txt
		# Plot it
		echo -9 0 9 0 | gmt plot -Jx? -Sv12p+s+e+p0.5p -Gred -W0.5p,-
		gmt plot line.txt -W3p
		gmt plot cross.txt -W1p
		# Sample it along profiles with -30 degree deviation to the line y = 0
		gmt subplot set 2 -A"-30 deviation, right side only"
		gmt grdtrack -Gcart.grd -C8/0.1/1+d-30+r -E-7/0/7/0 -Dline.txt > cross.txt
		# Plot it
		echo -9 0 9 0 | gmt plot -Jx? -Sv12p+s+e+p0.5p -Gred -W0.5p,-
		gmt plot line.txt -W3p
		gmt plot cross.txt -W1p
	gmt subplot end
gmt end show
