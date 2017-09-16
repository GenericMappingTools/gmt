#!/bin/bash
# Shows bug in grdblend when cutting across input grids
# The bottom region should equal the insert box on top.
ps=center.ps
gmt grdmath -R126/127/42/43 -I5m X Y ADD = LL.nc
gmt grdmath -R127/128/42/43 -I5m X Y ADD = LR.nc
gmt grdmath -R126/127/43/44 -I5m X Y ADD = UL.nc
gmt grdmath -R127/128/43/44 -I5m X Y ADD = UR.nc
gmt grdblend -R126.5/127.5/42.5/43.5 -I5m [LU][LR].nc -GCM.nc
gmt grdblend -R126/128/42/44 -I5m [LU][LR].nc -Gbig.nc
gmt makecpt -Cjet -T5292/5632 > t.cpt
gmt makecpt -Cjet -T168/172 > t.cpt
gmt grdimage CM.nc -Ct.cpt -JX4i -P -Bafg30 -K -Xc > $ps
gmt grdimage big.nc -Ct.cpt -J -P -Bafg30 -BWsNE -O -K -Y4.5i >> $ps
gmt grdinfo CM.nc -Ib | gmt psxy -Rbig.nc -J -W1p -O -A >> $ps
