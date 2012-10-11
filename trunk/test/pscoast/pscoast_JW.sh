#!/bin/bash
#	$Id$
#

ps=pscoast_JW.ps

pscoast -R-10/180/40:44:11.8/90 -JW30/10i -Ba20g10 -A0/0/1 -G221/204/170 -S153/221/255 -N1/0.125p,black,- -N3/0.125p,black,- -W0.125p,black -Xc -Yc -Dc > $ps
