#!/usr/bin/env bash
# Minimal example showing map clipping.
gmt begin clip ps
  gmt clip -R0/6/0/6 -Jx2.5c -W1p,blue << EOF
0 0
5 1
5 5
EOF
  gmt plot @tut_data.txt -Gred -Sc2c
  gmt psclip -C -B
gmt end show
