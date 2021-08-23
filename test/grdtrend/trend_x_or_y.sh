#!/usr/bin/env bash
#
#	Testing the +x and +y modifiers to grdtrend -N

gmt grdmath -R-1/1/-1/1 -I0.1 X = x.grd
gmt grdtrend x.grd -N3+x -Dlixo_flat_x.grd

gmt grdmath -R-1/1/-1/1 -I0.1 Y = y.grd
gmt grdtrend y.grd -N3+y -Dlixo_flat_y.grd

# Compute any difference between data and solution and if not 0 write it out
gmt grdmath x.grd lixo_flat_x.grd SUB 0 NAN = diffx.grd
gmt grdmath y.grd lixo_flat_y.grd SUB 0 NAN = diffy.grd
gmt grd2xyz diffx.grd -s > fail
gmt grd2xyz diffy.grd -s >> fail
