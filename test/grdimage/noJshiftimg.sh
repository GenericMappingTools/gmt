#!/usr/bin/env bash
# Plot images with different central meridian in rectangular projections
# DVC_TEST

gmt begin noJshiftimg ps
	gmt grdimage -JQ60/14c -B @earth_day_01d
	gmt grdimage -JQ-60/14c -B @earth_day_01d -Y8c
	gmt grdimage -JH270/14c @earth_day_01d -Y8c
gmt end show
