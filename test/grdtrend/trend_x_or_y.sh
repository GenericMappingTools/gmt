#!/usr/bin/env bash
#
#	Testing the +x and +y modifiers to grdtrend -N

gmt grdmath -R-1/1/-1/1 -I0.1 X = x.grd
gmt grdtrend x.grd -N2+x -Dlixo_flat_x.grd

gmt grdmath -R-1/1/-1/1 -I0.1 Y = y.grd
gmt grdtrend y.grd -N2+y -Dlixo_flat_y.grd

# Replace 0 with NaN and write out anything else as failure
gmt grdmath lixo_flat_x.grd 0 NAN = lixo_flat_x.grd
gmt grdmath lixo_flat_y.grd 0 NAN = lixo_flat_y.grd
gmt grd2xyz lixo_flat_x.grd -s > fail
gmt grd2xyz lixo_flat_y.grd -s >> fail
