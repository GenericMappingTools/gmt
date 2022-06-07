#!/usr/bin/env bash
# Demonstration of -w through a series of cycle plots
# This one shows how to wrap yearly and show data on a normalized year
gmt begin GMT_cycle_2
	gmt set GMT_THEME cookbook
	gmt plot @mississippi.txt -R0/1/0/50 -JX15c/7c -W0.25p,red -Bxaf -Byaf+l"10@+3@+ m@+3@+/s" -BWSrt+t"Mississippi river annual discharge" -i0,1+s1e-3 -wy
gmt end show
