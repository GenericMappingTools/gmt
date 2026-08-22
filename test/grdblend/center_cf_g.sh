#!/usr/bin/env bash
# Shows bug in grdblend when cutting across input grids
# The bottom region should equal the inset box on top.
# This is for the old netcdf format. Gridline-registration.
ps=center_cf_g.ps
gmt grdmath -R-2/0/-2/0 -I1 X Y ADD = LL.cdf=cf
gmt grdmath -R0/2/-2/0  -I1 X Y ADD = LR.cdf=cf
gmt grdmath -R-2/0/0/2  -I1 X Y ADD = UL.cdf=cf
gmt grdmath -R0/2/0/2   -I1 X Y ADD = UR.cdf=cf
gmt grdblend -R-1/1/-1/1 -I1 LL.cdf=cf LR.cdf=cf UL.cdf=cf UR.cdf=cf -GCM.nc
gmt grdblend -R-2/2/-2/2 -I1 LL.cdf=cf LR.cdf=cf UL.cdf=cf UR.cdf=cf -Gall.nc
gmt makecpt -Cjet -T-4/4 > t.cpt
gmt grdimage CM.nc -Ct.cpt -JX4i -P -Bafg2 -K -Xc > $ps
gmt grd2xyz CM.nc | gmt pstext -RCM.nc -JX4i -O -K -F+f16p+jCM+z%g -N -Gwhite >> $ps
gmt grdimage all.nc -Ct.cpt -JX4i -P -Bafg2 -BWsNE+t"Gridline old netCDF cf" -O -K -Y4.5i >> $ps
gmt grdinfo CM.nc -Ib | gmt psxy -Rall.nc -JX4i -W4p -O -K -A >> $ps
gmt grd2xyz all.nc | gmt pstext -Rall.nc -JX4i -O -F+f16p+jCM+z%g -N -Gwhite >> $ps
