#!/usr/bin/env bash
#		GMT EXAMPLE 09
#
# Purpose:	Make wiggle plot along track from geoid deflections
# GMT modules:	convert, wiggle, text, plot
#
gmt begin ex09
	gmt wiggle @tracks_09.txt -R185/250/-68/-42 -Jm0.35c -B -BWSne+ghoneydew -Gred+p \
		-Gblue+n -Z750c -Wthinnest -DjBR+w500+l@~m@~rad+o0.5c --FORMAT_GEO_MAP=dddF
	gmt plot @ridge_09.txt -Wthicker
	gmt plot @fz_09.txt -Wthinner,-
	# Take label from segment header and plot near coordinates of last record of each track
	gmt convert -El @tracks_09.txt | gmt text -F+f10p,Helvetica-Bold+a50+jRM+h -D-4p
gmt end show
