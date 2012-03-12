#!/bin/bash
#	$Id$
#
# Split a polyline into chunks

. functions.sh
header "Test splitxyz for splitting a polyline"

ps=split.ps
psxy $src/data.dat -R-15/-12/35/37.5 -JM10c -W5p,gray -B1 -P -K > $ps
splitxyz $src/data.dat -C45 -A45/15 -fg | psxy -R-15/-12/35/38 -JM10c -W1p -O >> $ps

pscmp
