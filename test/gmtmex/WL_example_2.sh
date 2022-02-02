#!/usr/bin/env bash
#
# Bourne shell replica of GMT/MEX example 2 (Japan trench)
# This is Figure 3 in Wessel & Luis, 2017
# Need JP.txt which we replicate here
# DVC_TEST
cat << EOF > JP.txt
# Trace of the Japan Trench
 143.0175   36.3400
 143.1533   36.5193
 143.2774   36.6477
 143.4073   36.8615
 143.5182   37.0583
 143.6350   37.2884
 143.7445   37.4783
 143.8438   37.7106
 143.8891   37.9295
 143.9299   38.1662
 143.9737   38.3595
 144.0161   38.5352
 144.0628   38.7093
 144.1197   38.9092
 144.1869   39.1233
 144.2044   39.3741
 144.2380   39.5575
 144.2263   39.7673
 144.2540   39.9282
 144.2818   40.0686
 144.3285   40.2042
 144.3985   40.3674
 144.4803   40.5503
 144.6117   40.7204
EOF
gmt begin WL_example_2 ps
  gmt grdcut @earth_relief_01m -R135/150/30/45 -GJP.nc
  gmt grdtrack JP.txt -GJP.nc -C300k/1k/25k -Sm+a+sstack.txt > profiles.txt
  gmt convert -Ef -T profiles.txt > area.txt
  gmt convert -El -T -Is profiles.txt >> area.txt
  gmt makecpt -Cearth -T-10000/4000
  gmt grdimage -R141/147/35/42 -JM6i -Baf JP.nc -I+a45+nt0.2
  gmt coast -W0.25p -Dh
  gmt plot -Ggreen@75 area.txt
  gmt plot -W1p JP.txt
  gmt plot -W0.25p,red profiles.txt
  rm -f JP.*
gmt end show
