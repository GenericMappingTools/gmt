#!/usr/bin/env bash
# Test implementation of giving width and/or height of color bar in percentages
# when -B is not used.
gmt begin scale_percent ps
	gmt makecpt -Cturbo -T0/10
	gmt basemap -R0/10/0/10 -JX15c -B
	gmt colorbar -DJTC+w40%/15% -B
	gmt colorbar -DJML+w9c/5% -B
	gmt colorbar -DJMR+w80%/0.6c -B
	gmt colorbar -DJBC+w70% -B
gmt end show
