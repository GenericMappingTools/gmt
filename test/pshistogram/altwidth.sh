#!/usr/bin/env bash
# Ensure the new -E option works as advertised
gmt begin altwidth ps
	gmt subplot begin 3x2 -Fs7c -SCb -SRl -R3/7/0/8 -BWSrt -A
		gmt subplot set -A"original"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p
		gmt subplot set -A"New x-width"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.1
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.1+o-0.05 -c
		gmt histogram  hist.txt -T0.2 -Ggreen -W0.5p -E0.1+o0.05
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.2c -c
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.2c+o-0.1c -c
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.2c+o-0.1c -c
		gmt histogram  hist.txt -T0.2 -Ggreen -W0.5p -E0.2c+o0.1c
	gmt subplot end
gmt end show
