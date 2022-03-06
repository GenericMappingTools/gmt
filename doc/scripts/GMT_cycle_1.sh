#!/usr/bin/env bash
# Demonstration of -w through a series of cycle plots
# This one shows the initial time series (no -w yet)
gmt begin GMT_cycle_1
	gmt set GMT_THEME cookbook
	gmt plot @mississippi.txt -JX15cT/7c -Bxaf -Byaf+l"10@+3@+ m@+3@+/s" -BWSrt+t"Mississippi river daily discharge" -W0.25p,red -i0,1+s1e-3
gmt end show
