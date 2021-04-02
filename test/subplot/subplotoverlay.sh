#!/usr/bin/env bash
# Test subplot -D in successfully overlaying two subplots
gmt begin subplotoverlay ps
	gmt subplot begin 3x2 -R0/8/0/6 -Fs8c/6c -M6p -A -SCl -SRb -Bafg1 -T"My Repeated Subplot"
		gmt basemap -c
		gmt basemap -c
		gmt basemap -c
		gmt basemap -c
		gmt basemap -c
		gmt basemap -c
	gmt subplot end
	# Now overlay data with same -B -C, -F, -M, -S and use -D to not draw frames again
	gmt subplot begin 3x2 -R0/8/0/6 -Fs8c/6c -M6p -SCl -SR -D
		echo 4 3 | gmt plot -Sc2c -Gred -c
		echo 4 3 | gmt plot -Sc2c -Ggreen -c
		echo 4 3 | gmt plot -Sc2c -Gblue -c
		echo 4 3 | gmt plot -Sc2c -Gred -c
		echo 4 3 | gmt plot -Sc2c -Ggreen -c
		echo 4 3 | gmt plot -Sc2c -Gblue -c
	gmt subplot end
gmt end show
