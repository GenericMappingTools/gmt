#!/usr/bin/env bash
gmt begin GMT_mercator ps
gmt set MAP_FRAME_TYPE fancy+
gmt coast -R0/360/-70/70 -Jm1.2e-2i -Bxa60f15 -Bya30f15 -Dc -A5000 -Gred 
gmt end
