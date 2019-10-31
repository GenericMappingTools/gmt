#!/usr/bin/env bash
# Test the variable widths and heights options for subplots
# This test uses a variable subplot dimensions
gmt begin vardims ps
  gmt subplot begin 3x2 -Fs2i,4i/2.5i,5i,1.25i -SRl+p -SCb -BWSne -M0 -AA+v -T"Variable dimensions"
    gmt basemap -R0/5/0/5 -B+gpink
    gmt basemap -R0/5/10/15 -B+gcyan -c
    gmt basemap -R0/5/10/15 -B+glightgray -c
    gmt basemap -R10/15/0/5 -B+gyellow -c
    gmt basemap -R10/15/10/15 -B+gpurple -c
    gmt basemap -R10/15/10/15 -B+gorange -c
  gmt subplot end
gmt end show
