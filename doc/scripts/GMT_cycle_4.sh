#!/usr/bin/env bash
# Demonstration of -w through a series of cycle plots
# This one shows how to apply wrapping to the y-coordinate
gmt begin GMT_cycle_4
	gmt set GMT_THEME cookbook
	gmt subplot begin 1x2 -Fs8c/10c -BWSrt -T"Mississippi river annual discharge" -A+jTR
		gmt plot @mississippi.txt -i1+s1e-3,0 -R0/50/0/1 -W0.25p,blue -Byaf+l"Normalized year" -Bxaf+l"10@+3@+ m@+3@+/s" -wy+c1 -c
		gmt histogram @mississippi.txt -R-3/9/0/8 -T1 -Gblue -W1p -Bxaf -Byaf+l"10@+6@+ m@+3@+/s" -Z0+w -i0,1+s1e-6 -A -wa -c  --FORMAT_TIME_PRIMARY_MAP=a
	gmt subplot end
gmt end show
