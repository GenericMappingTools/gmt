#!/usr/bin/env bash

gmt begin polarapex ps
  gmt subplot begin 4x3 -Fs2i -Bafg -M3p -X1.25c
    gmt basemap -R0/190/0/1 -JP? -c
    gmt basemap -R0/180/0/1 -JP? -c
    gmt basemap -R0/170/0/1 -JP? -c
    gmt basemap -R0/160/0/1 -c
    gmt basemap -R0/150/0/1 -c
    gmt basemap -R0/140/0/1 -c
    gmt basemap -R0/135/0/1 -c
    gmt basemap -R0/130/0/1 -c
    gmt basemap -R0/120/0/1 -c
    gmt basemap -R0/110/0/1 -c
    gmt basemap -R0/100/0/1 -c
    gmt basemap -R0/90/0/1 -c
  gmt subplot end
gmt end show
