#!/usr/bin/env bash
gmt begin GMT_map_frame_type
  gmt set GMT_THEME cookbook
  gmt set FONT_TAG 12p,Helvetica-Bold
  gmt subplot begin 2x3 -Fs4c/3.5c -M0.5c -R-5/5/-4/5 -A+JTC+o4p/10p
    gmt subplot set -Afancy
    gmt basemap -JM? -Baf -BWSen --MAP_FRAME_TYPE=fancy
    gmt subplot set -Afancy+
    gmt basemap -JM? -Baf -BWSen --MAP_FRAME_TYPE=fancy+
    gmt subplot set -Aplain
    gmt basemap -JM? -Baf -BWSen --MAP_FRAME_TYPE=plain
    gmt subplot set -Ainside
    gmt basemap -JX? -Baf -BWSen --MAP_FRAME_TYPE=inside
    gmt subplot set -Agraph
    gmt basemap -JX? -Baf -BWS --MAP_FRAME_TYPE=graph
    gmt subplot set -Agraph-origin
    gmt basemap -JX? -Baf -BWS --MAP_FRAME_TYPE=graph-origin
  gmt subplot end
gmt end show
