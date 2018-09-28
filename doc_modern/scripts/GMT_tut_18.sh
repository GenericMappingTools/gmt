#!/bin/bash
gmt grd2cpt @tut_bathy.nc -Cocean > bermuda.cpt
gmt grdview tut_bathy.nc -JM5i -JZ2i -p135/30 -Ba -Cbermuda.cpt -ps GMT_tut_18
