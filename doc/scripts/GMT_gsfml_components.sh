#!/bin/bash
# Plot the three building blocks G0, G1, G2 in a normalized way
gmt begin GMT_gsfml_components
	fzmodeler -D-30/30/0.1 -N1 -W10 -G0 -T-     | gmt plot -W1p,red -Bxafg5 -Byafg1 -R-30/30/-1.25/1.25 -JX15c/-5c -l"-G@-0@-"
	fzmodeler -D-30/30/0.1 -N1 -W10 -A1 -G1 -T- | gmt plot -W1p,green -JX15c/5c -l"G@-1@-"
	fzmodeler -D-30/30/0.1 -N1 -W10 -C1 -G2 -T- | gmt plot -W1p,blue -l"G@-2@-"
gmt end show
