#!/usr/bin/env bash
# Testing map directional roses for magnetics for positive declination as well
# Fixes issue in https://github.com/GenericMappingTools/gmt/issues/3328

ps=compass.ps
gmt psbasemap -R195/225/0/90 -JT210/6c -Tmg210/15+w8c+d20+t30/10/5/30/10/2+i0.02c+p0.02c+l -P -K -X8c -Y5c > $ps
gmt psbasemap -R -J -Tmg210/15+w8c+d-30+t30/10/5/30/10/2+i0.02c+p0.02c+l -Y13c -O >> $ps
