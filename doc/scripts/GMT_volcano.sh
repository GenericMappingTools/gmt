#!/bin/sh
#	$Id: GMT_volcano.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $

$AWK -f make_symbol < volcano.def > volcano.awk
chmod +x volcano.awk

echo "0 0 1" | $AWK -f volcano.awk | psxy -R-0.5/0.5/-0.5/0.5 -JX2i -P -Ba0.25g0.05WSne -W1p -M > GMT_volcano.ps
\rm -f volcano.awk
