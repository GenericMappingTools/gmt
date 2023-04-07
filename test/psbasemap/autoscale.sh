#!/usr/bin/env bash
# We had a bug when one axis was set to -0 and the other was negative
# Try all combinations of -JX[±]W/±0 and -JX±0/[±]H plus plain -JX[±]W
gmt begin autoscale
	gmt basemap -R0/10/0/5 -JX7c -B -X3c -Y1.5c
	echo "-JX7c" | gmt text -F+cCM+f18p
	gmt basemap -JX-7c -B -X8c
	echo "-JX-7c" | gmt text -F+cCM+f18p
	gmt basemap -JX7c/0 -B -X-8c -Y8c
	echo "-JX7c/0" | gmt text -F+cCM+f18p
	gmt basemap -JX-7c/0 -B -X8c
	echo "-JX-7c/0" | gmt text -F+cCM+f18p
	gmt basemap -JX7c/-0 -B -X-8c -Y4.5c
	echo "-JX7c/-0" | gmt text -F+cCM+f18p
	gmt basemap -JX-7c/-0 -B -X8c
	echo "-JX-7c/-0" | gmt text -F+cCM+f18p
	gmt basemap -JX0/3.5c -B -X-8c -Y4.5c
	echo "-JX0/3.5c" | gmt text -F+cCM+f18p
	gmt basemap -JX0/-3.5c -B -X8c
	echo "-JX0/-3.5c" | gmt text -F+cCM+f18p
	gmt basemap -JX-0/3.5c -B -X-8c -Y4.5c
	echo "-JX-0/3.5c" | gmt text -F+cCM+f18p
	gmt basemap -JX-0/-3.5c -B -X8c
	echo "-JX-0/-3.5c" | gmt text -F+cCM+f18p
gmt end show
