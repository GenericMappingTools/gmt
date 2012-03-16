#!/bin/bash
#	$Id$
#
# Split a polyline into chunks

header "Test splitxyz for splitting a polyline"

psxy data.dat -R-15/-12/35/37.5 -JM10c -W5p,gray -B1 -P -K > $ps
splitxyz data.dat -C45 -A45/15 -fg | psxy -R-15/-12/35/38 -JM10c -W1p -O >> $ps

pscmp
