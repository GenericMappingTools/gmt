#!/usr/bin/env bash
# Test the variable widths and heights options for subplots
# This test uses a fixed figure size and fractional dims
gmt begin varfracs ps
  gmt subplot begin 3x2 -Ff6.5i/9i+f1,2/1,2,0.5 -SRl+p -SCb -BWSne -M0 -A -T"Variable fractions"
  gmt psbasemap -R0/5/0/5 -B+gpink -c1,1
  gmt psbasemap -R10/15/0/5 -B+gyellow -c1,2
  gmt psbasemap -R0/5/10/15 -B+gcyan -c2,1
  gmt psbasemap -R10/15/10/15 -B+gpurple -c2,2
  gmt psbasemap -R0/5/10/15 -B+glightgray -c3,1
  gmt psbasemap -R10/15/10/15 -B+gorange -c3,2
  gmt subplot end
gmt end
