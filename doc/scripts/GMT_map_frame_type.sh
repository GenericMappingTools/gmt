#!/usr/bin/env bash
gmt begin GMT_map_frame_type
  gmt set GMT_THEME cookbook
  gmt set FONT_TITLE 18p
  gmt subplot begin 1x5 -Fs3.5c/3.5c -M0.4c -R0/10/0/10
    gmt basemap -JM? -Baf -BWSen+t"fancy" -c --MAP_FRAME_TYPE=fancy
    gmt basemap -JM? -Baf -BWSen+t"fancy+" -c --MAP_FRAME_TYPE=fancy+
    gmt basemap -JM? -Baf -BWSen+t"plain" -c --MAP_FRAME_TYPE=plain
    gmt basemap -JX? -Baf -BWSen+t"inside" -c --MAP_FRAME_TYPE=inside
    gmt basemap -JX? -Baf -BWS+t"graph" -c --MAP_FRAME_TYPE=graph
  gmt subplot end
gmt end show
