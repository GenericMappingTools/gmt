#!/usr/bin/env bash
# Show that contours in TM projection no longer wrap in y-direction
# https://github.com/GenericMappingTools/gmt/issues/2389
gmt begin TMcontours ps
	gmt grdmath -Rg -I2 116.40 39.90 SDIST = dist.nc
    gmt coast -R0/360/-80/80 -JT180/-30/6i -Glightgray -A1000 -Bag
	gmt grdcontour dist.nc -C1000 -Wc1p,red
gmt end show
