#!/usr/bin/env bash
gmt begin GMT_tut_15 ps
  gmt makecpt -Crainbow -T1000/5000
  gmt grdimage @tut_relief.nc -JM6i -Ba -BWSnE
  gmt colorbar -DJTC -I0.4 -Bxa -By+lm 
gmt end
