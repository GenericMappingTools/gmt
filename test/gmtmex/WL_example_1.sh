#!/usr/bin/env bash
#
# Bourne shell replica of GMT/MEX example 2 (Geologists Seamounts)
# This is Figure 2 in Wessel & Luis, 2017
# Needs the Geologists Seamounts data set from cache
gmt begin WL_example_1 ps
  gmt blockmedian -R158W/156:40W/18N/19:40N -I1m @geologists.txt > ship_1m.txt
  gmt surface -I1m ship_1m.txt -Gship_1m.nc -T0.2 -N1000 -C1e-6
  gmt mask -I1m ship_1m.txt -S5k -T -Glightgreen -JM6i -Baf -BWSne+gpink
  gmt grdcontour ship_1m.nc -Z0.001 -A1 -C.250 -Wa0.75p,black,- -Wc0.25p,black,-
  gmt plot -Ss0.05c -Gred ship_1m.txt
  gmt mask -I1m ship_1m.txt -S5k
  gmt grdcontour ship_1m.nc -Z0.001 -A1 -C.250
  gmt mask -C
  rm -f ship_1m.*
gmt end show
