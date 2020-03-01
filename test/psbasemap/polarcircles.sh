#!/usr/bin/env bash
# Test new implementation of -JP modifiers
gmt begin polarcircles ps
	gmt set FONT_ANNOT_PRIMARY 7p MAP_TICK_LENGTH_PRIMARY 2p MAP_ANNOT_OFFSET_PRIMARY 2p FONT_TITLE 12p
	gmt subplot begin 4x3 -Fs2i -BWSNE -Bxa30fg -Byafg -M12p -Y0.5i
		gmt basemap -R0/300/0.4/1 -JP? -c -B+t"Default"
		gmt basemap -R0/270/0.4/1 -JP?+f -c -B+t"\053f"
		gmt basemap -R0/300/0/50 -JP?+fe -c -B+t"\053fe"
		gmt basemap -R0/300/1000/3000 -JP?+fp -c -B+t"\053fp"
		gmt basemap -R20/260/1000/3000 -JP?+z -c -B+t"\053z"
		gmt basemap -R0/300/0/2891 -JP?+zp -c -B+t"\053zp"
		gmt basemap -R0/300/0.4/1 -JP?+a -c -B+t"\053a"
		gmt basemap -R0/300/0.4/1 -JP?+t30 -c -B+t"\053t30"
		gmt basemap -R0/300/0.4/1 -JP?+a+t30 -c -B+t"\053a\053t30"
		gmt basemap -R0/270/0/1 -JP? -c -B+t"Default"
		gmt basemap -R0/300/0/1 -JP?+r4c -c -B+t"\053r4c"
		gmt basemap -R0/300/0/1 -JP?+f+r4c -c -B+t"\053f\053r4c"
	gmt subplot end
gmt end show
