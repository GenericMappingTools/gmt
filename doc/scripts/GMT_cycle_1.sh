#!/usr/bin/env bash
# Demonstration of -w through a series of cycle plots
gmt begin GMT_cycle_1
	gmt plot @mississippi.txt -JX6iT/3i -Bxaf -Byaf+l"10@+3@+ m@+3@+/s" -BWSrt+t"Mississippi river daily discharge" -W0.25p,red -i0,1+s1e-3
gmt end show
