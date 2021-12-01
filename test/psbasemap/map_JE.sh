#!/usr/bin/env bash
# Check gridlines for non-global -JE map
#
# MAP_ANNOT_OBLIQUE is set to use older classic settings
# MAP_ANNOT_MIN_ANGLE is set to prevent OS dependent failures due to rounding
#
ps=map_JE.ps

gmt psbasemap -B10g10 -Je-70/-90/1:10000000 -R-95/-75/-60/-55r -P -Xc --MAP_ANNOT_OBLIQUE=anywhere --MAP_ANNOT_MIN_ANGLE=19 > $ps
