#!/usr/bin/env bash
# Global pixel grids offset by half a grid increment used to fail but now passes
# The same for gridline-registered grids work.
# Here, q0.nc is fine but q.nc is shifted by 1/2 dx.

ps=oneincshift.ps
gmt grdmath -Rg -I1 -r Y COSD 20 MUL X SIND MUL = q0.nc
gmt grdedit -S -R-0:30/359:30/-90/90 q0.nc -Gq.nc
gmt makecpt -Chaxby -T-20/20/2 > sst.cpt
gmt grdimage q0.nc -JG135/45/90/4.5i -B60g15 -Csst.cpt -P -K -Xc > $ps
gmt grdimage q.nc -J -B60g15 -Csst.cpt -O -Y4.75i >> $ps
