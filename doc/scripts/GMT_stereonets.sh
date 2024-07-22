#!/usr/bin/env bash
gmt begin GMT_stereonets
	gmt basemap -R0/360/-90/90 -JA0/0/1.75i -Bg15
	echo "180 -90 SCHMIDT" | gmt text -N -D0/-0.2c -F+f12p,Helvetica-Bold+jTC
	gmt basemap -JS0/0/1.75i -Bg15 -X2.75i
	echo "180 -90 WULFF" | gmt text -N -D0/-0.2c -F+f12p,Helvetica-Bold+jTC
gmt end show
