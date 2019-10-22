#!/usr/bin/env bash
gmt begin GMT_tut_1
	gmt basemap -R10/70/-3/8 -JX4i/3i -B -B+glightred+t"My first plot"
gmt end show
