#!/bin/bash
#	$Id$
#

ps=pscoast_JW.ps

gmt pscoast -R-10/180/40:44:11.8/90 -JW30/10i -Ba20g10 -A0/0/2 -Glightbrown -Slightblue -W0.125p,black -Xc -Yc -Dc > $ps
