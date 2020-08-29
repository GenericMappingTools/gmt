#!/usr/bin/env bash
# Test for using standard latitude in Mercator for relative coordinates
# See https://github.com/GenericMappingTools/gmt/issues/2287

ps=merc_origin.ps

gmt psbasemap -JM173/-42/16c -R-200/200/-200/200+uk -Baf -BWSne -P --MAP_FRAME_TYPE=plain -K > $ps
echo 173 -42 | gmt psxy -R -J -O -S+1c -W1p >> $ps
