#!/usr/bin/env bash

gmt begin basemap png
	gmt basemap -R0/10/0/10 -JM10c -Baf
gmt end show
