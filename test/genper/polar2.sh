#!/bin/sh
# Check that odd gridline clipping due to bad mapjump call dont resurface
ps=polar2.ps
lon0=280.969616634
lat0=58.7193421224
gmt pscoast -Rg -JG$lon0/$lat0/2513/0/0/0/200/180/6i -Ba30f10g10/a5f5g5 -Glightbrown -Sazure1 -A1000 -Wfaint -Xc -P --MAP_ANNOT_MIN_SPACING=0.2i > $ps
