#!/usr/bin/env bash
gmt begin GMT_tut_19 ps
  gmt makecpt -Ctopo -T1000/5000
  gmt grdgradient @tut_relief.nc -Ne0.8 -A100 -fg -Gus_i.nc
  gmt grdview tut_relief.nc -JM4i -p135/35 -Qi50 -Ius_i.nc -C -Ba -JZ0.5i
gmt end

