#!/usr/bin/env bash
# Check gridlines for non-global -JE map
#
# GMT_KNOWN_FAILURE_WINDOWS
#
ps=map_JE.ps

gmt psbasemap -B10g10 -Je-70/-90/1:10000000 -R-95/-75/-60/-55r -P -Xc > $ps
