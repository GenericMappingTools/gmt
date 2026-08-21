#!/usr/bin/env bash
# Test horizontal and vertical Cartesian scales in basemap
ps=scales.ps
gmt psbasemap -R10000/30000/5000/6000 -JX6i/9i -Baf -LjCB+w4000+lfurlong+o0/0.5i -P -K -Xc --FORMAT_FLOAT_MAP="%'g" > $ps
gmt psbasemap -R -J -O -LjLM+w200+ls+v >> $ps
