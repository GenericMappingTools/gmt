#!/bin/bash
gmt begin GMT_tut_15 ps
gmt makecpt -Crainbow -T1000/5000 > topo.cpt
gmt grdimage @tut_relief.nc -JM6i -Ba -BWSnE -Ctopo.cpt 
gmt colorbar -DJTC -Rtut_relief.nc -Ctopo.cpt -I0.4 -Bxa -By+lm 
gmt end
