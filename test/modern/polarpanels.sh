#!/usr/bin/env bash
gmt begin polarpanels ps
  gmt subplot begin 2x2 -Fs5c
    gmt basemap -JPa?/45 -R0/90/0/1 -Bxa45f -c1,1
    gmt basemap -JPa?/45 -R0/90/0/1 -Bxa45f -c1,2
    gmt basemap -JPa?/45 -R0/90/0/1 -Bxa45f -c2,1
    gmt basemap -JPa?/45 -R0/90/0/1 -Bxa45f -c2,2
  gmt subplot end
gmt end