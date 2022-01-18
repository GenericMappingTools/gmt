#!/usr/bin/env bash
# Ensure the bug reported by https://github.com/GenericMappingTools/gmt/issues/6217 is gone
# Black dots are input points to be connected by blue line
gmt begin line_shortening
	cat <<- EOF > t.txt
	2 4
	2 1
	2 3.5
	EOF
	gmt plot -R0/5/0/5 -JX8c -Ba1g1 -W1p,blue t.txt
	gmt plot -Sc3p -Gblack t.txt
gmt end show
