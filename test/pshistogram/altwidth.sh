#!/usr/bin/env bash
# Ensure the new -E option works as advertised
gmt begin altwidth ps
	gmt subplot begin 3x2 -Fs7c -Scb -Srl -R3/7/0/8 -BWSrt -A
		gmt subplot set -A"No -E"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p
		gmt subplot set -A"w=0.1"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.1
		gmt subplot set -A"w=0.1 |off|=0.05"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.1+o-0.05
		gmt histogram  hist.txt -T0.2 -Ggreen -W0.5p -E0.1+o0.05
		gmt subplot set -A"w=0.1c"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.1c
		gmt subplot set -A"w=0.1c off=-0.05c"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.1c+o-0.05c
		gmt subplot set -A"w=0.1c |off|=0.05c"
		gmt histogram  hist.txt -T0.2 -Gred -W0.5p -E0.1c+o-0.05c
		gmt histogram  hist.txt -T0.2 -Ggreen -W0.5p -E0.1c+o0.05c
	gmt subplot end
gmt end show
