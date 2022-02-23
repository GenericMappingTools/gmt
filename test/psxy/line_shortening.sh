#!/usr/bin/env bash
# Ensure the bug reported by https://github.com/GenericMappingTools/gmt/issues/6217 is gone
# Black dots are input points to be connected by blue line
gmt begin line_shortening ps
	cat <<- EOF > v.txt
	2 4
	2 1
	2 3.5
	EOF
	cat <<- EOF > h.txt
	1	0.5
	4	0.5
	3.5	0.5
	EOF
	gmt plot -R0/5/0/5 -JX8c -Ba1g1 -W1p,blue v.txt h.txt
	gmt plot -Sc3p -Gblack v.txt h.txt
gmt end show
