#!/usr/bin/env bash
#		GMT EXAMPLE 06
#
# Purpose:	Make standard and polar histograms
# GMT modules:	histogram, rose, subplot
#
gmt begin ex06
	gmt subplot begin 2x1 -A+JTL+o0.8c -Fs15c/9c -M1c
		gmt histogram -Bx+l"Topography (m)" -By+l"Frequency"+u" %" -BWSne+t"Histograms"+glightblue @v3206_06.txt \
			-R-6000/0/0/30 -Gorange -W1p -Z1 -T250 -c0
		gmt rose @fractures_06.txt -: -A10r -S -Gorange -R0/1/0/360 -Bg -B+glightblue -W1p -c1
	gmt subplot end
gmt end show
