#!/bin/bash
gmt makecpt -Ctopo -T1000/5000 > t.cpt
gmt grdgradient @tut_relief.nc -Ne0.8 -A100 -fg -Gus_i.nc
gmt grdview tut_relief.nc -JM6i -p135/35 -Qi50 -Ius_i.nc -Ct.cpt -Ba -JZ0.5i -ps GMT_tut_19
