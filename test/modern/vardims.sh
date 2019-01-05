#!/usr/bin/env bash
# Test the variable widths and heights options for subplots
# This test uses a variable subplot dimensions
gmt begin vardims ps
  gmt subplot begin 3x2 -Fs2i,4i/2.5i,5i,1.25i -SRl+p -SCb -BWSne -M0 -AA+v -T"Variable dimensions"
  gmt basemap -R0/5/0/5 -B+gpink -c1,1
  gmt basemap -R10/15/0/5 -B+gyellow -c1,2
  gmt basemap -R0/5/10/15 -B+gcyan -c2,1
  gmt basemap -R10/15/10/15 -B+gpurple -c2,2
  gmt basemap -R0/5/10/15 -B+glightgray -c3,1
  gmt basemap -R10/15/10/15 -B+gorange -c3,2
  gmt subplot end
gmt end
