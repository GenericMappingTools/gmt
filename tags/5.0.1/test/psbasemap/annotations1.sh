#!/bin/bash
#
#	$Id$

. functions.sh
header "Test psbasemap's ddd:mm:ssF annotation formats"

psbasemap="psbasemap -JX3id/2.5id --FONT_ANNOT_PRIMARY=10p"
$psbasemap -R-25/25/-19/23 -B10WSne -P -K --FORMAT_GEO_MAP=dddF -Xf1i -Yf1i > $ps
$psbasemap -R-1.5/1.5/-1.2/1.5 -B0.5wSnE -O -K  --FORMAT_GEO_MAP=ddd.xF -Xf4.5i >> $ps
$psbasemap -R-1.05/1.05/-1.1/1.3 -B30mWSne -O -K --FORMAT_GEO_MAP=ddd:mmF -Xf1i -Yf4.25i >> $ps
$psbasemap -R-0:00:50/0:01:00/-0:01:00/0:01:00 -B0.5mwSnE -O -K  --FORMAT_GEO_MAP=ddd:mm.xF -Xf4.5i >> $ps
$psbasemap -R-0:00:30/0:00:30/-0:01:00/0:01:00 -B30sWSne -O -K --FORMAT_GEO_MAP=ddd:mm:ssF -Xf1i -Yf7.5i >> $ps
$psbasemap -R-0:00:04/0:00:05/-0:00:05/0:00:05 -B2.5swSnE -O --FORMAT_GEO_MAP=ddd:mm:ss.xF -Xf4.5i >> $ps

pscmp
