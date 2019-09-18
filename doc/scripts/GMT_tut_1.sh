#!/usr/bin/env bash
gmt begin GMT_tut_1 ps
  gmt basemap -R10/70/-3/8 -JX4i/3i -Ba -B+glightred+t"My first plot"
gmt end
