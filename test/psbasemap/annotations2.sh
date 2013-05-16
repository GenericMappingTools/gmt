#!/bin/bash
#
#	$Id$

ps=annotations2.ps

basemap="gmt psbasemap -JX3id/2.5id --FONT_ANNOT_PRIMARY=10p"
$basemap -R-25/25/-19/23 -B10 -BWSne -P -K --FORMAT_GEO_MAP=ddd -Xf1i -Yf1i > $ps
$basemap -R-1.5/1.5/-1.2/1.5 -B0.5 -BwSnE -O -K  --FORMAT_GEO_MAP=ddd.x -Xf4.5i >> $ps
$basemap -R-1.05/1.05/-1.1/1.3 -B30m -BWSne -O -K --FORMAT_GEO_MAP=ddd:mm -Xf1i -Yf4.25i >> $ps
$basemap -R-0:00:50/0:01:00/-0:01:00/0:01:00 -B0.5m -BwSnE -O -K  --FORMAT_GEO_MAP=ddd:mm.x -Xf4.5i >> $ps
$basemap -R-0:00:30/0:00:30/-0:01:00/0:01:00 -B30s -BWSne -O -K --FORMAT_GEO_MAP=ddd:mm:ss -Xf1i -Yf7.5i >> $ps
$basemap -R-0:00:04/0:00:05/-0:00:05/0:00:05 -B2.5s -BwSnE -O --FORMAT_GEO_MAP=ddd:mm:ss.x -Xf4.5i >> $ps

