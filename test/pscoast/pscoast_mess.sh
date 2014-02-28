#!/bin/bash
#	$Id$
#

ps=pscoast_mess.ps

gmt pscoast -JL153/-28/-26/-27/6i -Ba1 -BWSne -P -K -R151.5/-28.5/154.0/-27r -Dh -A20/1/1 -W0.5p -Glightgreen -Slightblue > $ps
gmt pscoast -J -Ba1 -BWSne -O -R -Di -A20/1/1 -W0.5p -Glightgreen -Slightblue -Y5i >> $ps
