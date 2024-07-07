#!/usr/bin/env bash
# Plot Europe on ESA liked Lamber Equal Area EPSG 3035
# Test that the inmplicit false Eastings and Northings do not screw the plot.
ps=proj4_nox0y0.ps

gmt pscoast  -R-23.8/24/72/59+r -J3035 -A1000/0/2 -Dc -W0.5 -Ba -P > $ps
