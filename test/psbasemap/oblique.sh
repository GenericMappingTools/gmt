#!/usr/bin/env bash

# Fixed 11/06/2015 P. Wessel: Treated as a special case where we
# have a stray horizontal line streaking across the entire map when
# the projection is oblique mercator.  The fix is implemented in
# GMT_plot_line in gmt_plot.c

ps=oblique.ps

gmt set MAP_ANNOT_OBLIQUE 14 MAP_ANNOT_MIN_SPACING 0.5i
gmt psbasemap -R-100/100/-60/60 -JOa1/0/45/5.5i -B30g30 -P -K -Xc > $ps
gmt psbasemap -R -JOa0/0.1/45/5.5i -B30g30 -O -Y5i >> $ps
