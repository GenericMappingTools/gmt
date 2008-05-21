#!/bin/sh
#
#	$Id: annotations2.sh,v 1.2 2008-05-21 19:05:50 guru Exp $

ps=annotations2.ps

. ../functions.sh
header "Test psbasemap's ddd:mm:ss annotation formats"

psbasemap="psbasemap -JX3id/2.5id --ANNOT_FONT_SIZE=10"
$psbasemap -R-25/25/-19/23 -B10WSne -P -K --PLOT_DEGREE_FORMAT=ddd -Xa1i -Ya1i > $ps
$psbasemap -R-1.5/1.5/-1.2/1.5 -B0.5wSnE -O -K  --PLOT_DEGREE_FORMAT=ddd.x -Xa4.5i -Ya1i >> $ps
$psbasemap -R-1.05/1.05/-1.1/1.3 -B30mWSne -O -K --PLOT_DEGREE_FORMAT=ddd:mm -Xa1i -Ya4.25i >> $ps
$psbasemap -R-0:00:50/0:01:00/-0:01:00/0:01:00 -B0.5mwSnE -O -K  --PLOT_DEGREE_FORMAT=ddd:mm.x -Xa4.5i -Ya4.25i >> $ps
$psbasemap -R-0:00:30/0:00:30/-0:01:00/0:01:00 -B30cWSne -O -K --PLOT_DEGREE_FORMAT=ddd:mm:ss -Xa1i -Ya7.5i >> $ps
$psbasemap -R-0:00:04/0:00:05/-0:00:05/0:00:05 -B2.5cwSnE -O -K --PLOT_DEGREE_FORMAT=ddd:mm:ss.x -Xa4.5i -Ya7.5i >> $ps
psxy -R -J -O /dev/null >> $ps

rm -f .gmtcommands4

pscmp
