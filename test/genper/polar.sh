#!/bin/sh
# Check that odd gridline clipping due to bad mapjump call dont resurface
ps=polar.ps
lon0=210.651901558
lat0=78.8472134393
gmt pscoast -Rg -JG$lon0/$lat0/1900/0/0/0/170/180/6i -Ba30f10g10/a5f5g5 -Glightbrown -A500 -Wfaint -Xc -P --MAP_ANNOT_MIN_SPACING=0.2i > $ps
