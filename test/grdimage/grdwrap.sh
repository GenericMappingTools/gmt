#!/bin/bash
#
#	$Id$

ps=grdwrap.ps

gmt grdmath -Rg -I1 X SIND Y COSD MUL = t.nc=ns/0.0001
gmt makecpt -Cpolar -T-1/1/0.01 > t.cpt

gmt grdimage t.nc -Ct.cpt -JQ-147/6i -B30 -K > $ps
gmt grdimage t.nc -Ct.cpt -JQ-147.533/6i -B30 -O -Y3.75i >> $ps

