#!/usr/bin/env bash
gmt begin GMT_colorbar
	gmt makecpt -T-15/15 -Cpolar
	gmt basemap -R0/20/0/1 -JM5i -BWse -B
	gmt colorbar -C -Bx+u"@." -By+l@~D@~T -DJBC+e
gmt end show
