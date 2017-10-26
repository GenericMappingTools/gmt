#!/bin/bash
#		GMT EXAMPLE 09
#		$Id$
#
# Purpose:	Make wiggle plot along track from geoid deflections
# GMT modules:	gmtconvert, pswiggle, pstext, psxy
# Unix progs:	
#
ps=example_09.ps
gmt pswiggle @tracks_09.txt -R185/250/-68/-42 -K -Jm0.13i -Ba10f5 -BWSne+g240/255/240 -G+red \
	-G-blue -Z2000 -Wthinnest -DjBR+l500+u@~m@~rad+o0.2i --FORMAT_GEO_MAP=dddF > $ps
gmt psxy -R -J -O -K @ridge_09.txt -Wthicker >> $ps
gmt psxy -R -J -O -K @fz_09.txt -Wthinner,- >> $ps
# Take label from segment header and plot near coordinates of last record of each track
gmt convert -El @tracks_09.txt | gmt pstext -R -J -F+f10p,Helvetica-Bold+a50+jRM+h \
	-D-0.05i/-0.05i -O >> $ps
