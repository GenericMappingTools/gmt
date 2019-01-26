#!/usr/bin/env bash
gmt begin GMT_-B_slanted ps
gmt basemap -R2000/2020/35/45 -JX12c -Bxa2f+a-30 -BS
gmt end
