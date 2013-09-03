#!/bin/bash
# $Id$

ps=vectors.ps
gmt grdmath -Rg -I30 -r 0.5 Y COSD ADD = r.nc
gmt grdmath -Rg -I30 -r X = az.nc
gmt makecpt -T0.5/1.5/0.1 -Z > t.cpt

gmt grdvector r.nc az.nc -A -Ct.cpt -JG45/45/4.5i -Q0.2i+e -W2p -Si2500 -P -K -B30g30 -Xc -Y0.75i --MAP_VECTOR_SHAPE=0.5 > $ps
gmt grdvector r.nc az.nc -A -Ct.cpt -J -W2p -Si2500 -O -B30g30 -Y5i >> $ps
#rm -f az.nc r.nc t.cpt
