#!/bin/bash
#
#	$Id$

ps=annotations3.ps

basemap="gmt psbasemap -R0/1000/0/1000 -JX2i/2i -B200f100 -BWESN -Bx+lhorizontal -By+lvertical --FONT_ANNOT_PRIMARY=10p --FONT_LABEL=16p"
$basemap --MAP_ANNOT_ORTHO=we -Xf1i -Yf1i -P -K > $ps
$basemap --MAP_ANNOT_ORTHO=sn -Xf4.5i -O -K >> $ps
$basemap --MAP_ANNOT_ORTHO=wesn -Xf1i -Yf4.5i -O -K >> $ps
$basemap --MAP_ANNOT_ORTHO= -Xf4.5i -O >> $ps

