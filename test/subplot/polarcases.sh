#!/usr/bin/env bash
# Test new implementation of -JP modifiers
gmt begin polarcases ps
	gmt subplot begin 4x3 -Fs3i -BWSNE -Baf -M3p
		gmt basemap -R0/120/0.4/1 -JP? -c -B+t"Default"
		gmt basemap -R0/120/0.4/1 -JP?+f -c -B+t"\053f"
		gmt basemap -R0/120/0/50 -JP?+fe -c -B+t"\053fe"
		gmt basemap -R0/120/1000/3000 -JP?+fp -c -B+t"\053fp"
		gmt basemap -R0/120/1000/3000 -JP?+z -c -B+t"\053z"
		gmt basemap -R30/180/0.4/1 -JP? -c -B+t"Default"
		gmt basemap -R30/180/0.4/1 -JP?+a -c -B+t"\053a"
		gmt basemap -R30/180/0.4/1 -JP?+r30 -c -B+t"\053r30"
		gmt basemap -R30/180/0.4/1 -JP?+a+r30 -c -B+t"\053a\053r30"
		gmt basemap -R0/120/0/1 -JP? -c -B+t"Default"
		gmt basemap -R0/120/0/1 -JP?+o4c -c -B+t"\053o4c"
		gmt basemap -R0/120/0/1 -JP?+f+o4c -c -B+t"\053f\053o4c"
	gmt subplot end
gmt end show
