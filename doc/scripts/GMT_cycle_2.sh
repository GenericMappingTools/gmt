#!/usr/bin/env bash
# Demonstration of -w through a series of cycle plots
gmt begin GMT_cycle_2
	gmt plot @mississippi.txt -R0/1/0/50 -JX6i/3i -W0.25p,red -Bxaf -Byaf+l"10@+3@+ m@+3@+/s" -BWSrt+t"Mississippi river annual discharge" -i0,1+s1e-3 -wy
gmt end show
