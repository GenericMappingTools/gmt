#!/bin/bash
#		GMT EXAMPLE 09
#		$Id$
#
# Purpose:	Make wiggle plot along track from geoid deflections
# GMT progs:	gmtconvert, pswiggle, pstext, psxy
# Unix progs:	$AWK, ls, paste, rm
#
ps=example_09.ps
pswiggle track_*.xys -R185/250/-68/-42 -U"Example 9 in Cookbook" -K -Jm0.13i \
	-Ba10f5 -BWSne+g240/255/240 -G+red -G-blue -Z2000 -Wthinnest -S240/-67/500/@~m@~rad \
	--FORMAT_GEO_MAP=dddF > $ps
psxy -R -J -O -K ridge.xy -Wthicker >> $ps
psxy -R -J -O -K fz.xy -Wthinner,- >> $ps
# Make label file from last record of each track
gmtconvert -El track_*.xys > tmp
# Extract track number from file name
ls -1 track_*.xys | $AWK -F. '{print $2}' > tracks.lis
# Combine the two to make input for labels
paste tmp tracks.lis | $AWK '{print $1, $2, $4}' \
	| pstext -R -J -F+f10p,Helvetica-Bold+a50+jRM -D-0.05i/-0.05i -O >> $ps
rm -f tmp tracks.lis
