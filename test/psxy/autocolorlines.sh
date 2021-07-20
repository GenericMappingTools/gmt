#!/usr/bin/env bash
# Test auto-color sequencing of pens via CPT or color list

gmt begin autocolorlines ps
	gmt subplot begin 2x1 -Fs16c/0 -R-50/0/-10/20 -JM16c -Sc -Sr -Bwesn
		gmt plot @fz_07.txt -W0.25p,auto -c
		gmt plot @fz_07.txt -W0.25p,auto -c --COLOR_SET=red,green,blue
	gmt subplot end
gmt end show
