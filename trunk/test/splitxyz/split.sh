#!/bin/bash
#	$Id: split.sh,v 1.2 2011-06-21 17:30:42 remko Exp $
#
# Split a polyline into chunks

. ../functions.sh
header "Test splitxyz for splitting a polyline"

ps=split.ps
psxy data.dat -R-15/-12/35/37.5 -JM10c -W5p,gray -B1 -P -K > $ps
splitxyz data.dat -C45 -A45/15 -gd6k -fg | psxy -R-15/-12/35/38 -JM10c -W1p -O >> $ps

pscmp
