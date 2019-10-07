#!/usr/bin/env bash
# Testing gmt grdsample over a periodic grid boundary
# Problem was issue # 1086
ps=straddle.ps
gmt grdcut @earth_relief_04m -R179/196/49/61 -Gc.nc
gmt grdsample @earth_relief_04m -I0.2/0.2 -R179/196/49/61 -Gs.nc
gmt grdimage c.nc -JM3.5i -P -K -Xc -Baf -BWSne -Cjet > $ps
gmt grdimage s.nc -J -O -Y4.8i -Baf -BWSne -Cjet >> $ps
