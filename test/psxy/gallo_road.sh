#!/usr/bin/env bash
#
# Demonstrate decorated line with EPS symbol
# Showing how the chicken (well, gallo) crossed the busy road
cat > path.txt << END
-2		-1.8
-1.5	-1.5
 0		1
 1.5	1.2
 2		1.7
END

gmt begin gallo_road
	gmt plot -R-2/2/-2/2 -JX16c -W40p,lightgray <<- EOF
	-2	2.4
	2	-2.4
	EOF
	gmt plot -W2p,- <<- EOF
	-2	2.4
	2	-2.4
	EOF
	gmt plot path.txt -W4p,black,8_4 -Bafg1 -BWSrt+t"How 'gallo' crossed road"
	gmt plot path.txt -S~n8:+i+sk@gallo/2c
gmt end show
