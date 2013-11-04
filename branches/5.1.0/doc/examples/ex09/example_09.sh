#!/bin/bash
#		GMT EXAMPLE 09
#		$Id$
#
# Purpose:	Make wiggle plot along track from geoid deflections
# GMT progs:	gmtconvert, pswiggle, pstext, psxy
# Unix progs:	
#
ps=example_09.ps
gmt pswiggle tracks.txt -R185/250/-68/-42 -K -Jm0.13i -Ba10f5 -BWSne+g240/255/240 -G+red \
	-G-blue -Z2000 -Wthinnest -S240/-67/500/@~m@~rad --FORMAT_GEO_MAP=dddF > $ps
gmt psxy -R -J -O -K ridge.xy -Wthicker >> $ps
gmt psxy -R -J -O -K fz.xy -Wthinner,- >> $ps
# Take label from segment header and plot near coordinates of last record of each track
gmt gmtconvert -El tracks.txt | gmt pstext -R -J -F+f10p,Helvetica-Bold+a50+jRM+h \
	-D-0.05i/-0.05i -O >> $ps
