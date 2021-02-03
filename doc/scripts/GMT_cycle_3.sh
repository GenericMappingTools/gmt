#!/usr/bin/env bash
# Demonstration of -w through a series of cycle plots
gmt begin GMT_cycle_3 ps
	gmt histogram @mississippi.txt -R0/12/0/8 -JX6i/3i -T1 -Gred -W1p -Bxaf -Byaf+l"10@+6@+ m@+3@+/s" -BWSrt+t"Average monthly discharge" -Z0+w -i0,1+s1e-6 -wo
gmt end show
