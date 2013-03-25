#!/bin/sh
# $Id$

ps=vectors.ps
grdmath -Rg -I30 -r 0.5 Y COSD ADD = r.nc
grdmath -Rg -I30 -r X = az.nc
makecpt -T0.5/1.5.0.1 -Z > t.cpt

grdvector r.nc az.nc -A -Ct.cpt -JG45/45/4.5i -Q0.15i+e+n0.5 -W1p -S3i -P -K -B30g30 -Xc -Y0.75i > $ps
grdvector r.nc az.nc -A -Ct.cpt -J -W1p -S3i -O -B30g30 -Y5i >> $ps
