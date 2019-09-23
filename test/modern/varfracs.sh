#!/usr/bin/env bash
# Test the variable widths and heights options for subplots
# This test uses a fixed figure size and fractional dims
gmt begin varfracs ps
  gmt subplot begin 3x2 -Ff6.5i/9i+f1,2/1,2,0.5 -SRl+p -SCb -BWSne -M0 -A -T"Variable fractions"
    gmt basemap -R0/5/0/5 -B+gpink
    gmt basemap -R10/15/0/5 -B+gyellow -c
    gmt basemap -R0/5/10/15 -B+gcyan -c
    gmt basemap -R10/15/10/15 -B+gpurple -c
    gmt basemap -R0/5/10/15 -B+glightgray -c
    gmt basemap -R10/15/10/15 -B+gorange -c
  gmt subplot end
gmt end show
