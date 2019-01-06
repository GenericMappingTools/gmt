#!/usr/bin/env bash
#
gmt grd2cpt @tut_bathy.nc -Cocean > bermuda.cpt
gmt grdview tut_bathy.nc -JM5i -P -JZ2i -p135/30 -Ba -Cbermuda.cpt > GMT_tut_18.ps
