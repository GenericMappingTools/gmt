#!/usr/bin/env bash
gmt begin GMT_cyclic
	gmt makecpt -T0/100 -Cjet -Ww
	gmt basemap -R0/20/0/1 -JM5i -BWse -Baf
	gmt colorbar -C -Baf -DJBC 
gmt end show
