#!/bin/bash
# Shows bug in grdblend when cutting across input grids
# The bottom region should equal the insert box on top.
# This is for the default new netCDF grid format
ps=center_nf.ps
gmt grdmath -R-2/0/-2/0 -I1 -r X Y ADD = LL.nc
gmt grdmath -R0/2/-2/0  -I1 -r X Y ADD = LR.nc
gmt grdmath -R-2/0/0/2  -I1 -r X Y ADD = UL.nc
gmt grdmath -R0/2/0/2   -I1 -r X Y ADD = UR.nc
gmt grdblend -R-1/1/-1/1 -I1 -r LL.nc LR.nc UL.nc UR.nc -GCM.nc
gmt grdblend -R-2/2/-2/2 -I1 -r LL.nc LR.nc UL.nc UR.nc -Gbig.nc
gmt makecpt -Cjet -T-4/4 > t.cpt
gmt grdimage CM.nc -Ct.cpt -JX4i -P -Bafg2 -K -Xc > $ps
gmt grd2xyz CM.nc | gmt pstext -RCM.nc -J -O -K -F+f16p+jCM >> $ps
gmt grdimage big.nc -Ct.cpt -J -P -Bafg2 -BWsNE+t"New netCDF nf" -O -K -Y4.5i >> $ps
gmt grdinfo CM.nc -Ib | gmt psxy -Rbig.nc -J -W4p -O -K -A >> $ps
gmt grd2xyz big.nc | gmt pstext -R -J -O -F+f16p+jCM  >> $ps
