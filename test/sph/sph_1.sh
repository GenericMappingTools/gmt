#!/bin/bash
#
#       $Id$

header "Testing sphinterpolate I"

makecpt -Crainbow -T-7000/15000/1000 -Z > tt.cpt
sphinterpolate mars370.txt -Rg -I1 -Q0 -Gtt.nc
grdimage tt.nc -JH0/4.5i -B30g30:."-Q0": -Ctt.cpt -X0.8i -Y5.5i -K --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p > $ps
sphinterpolate mars370.txt -Rg -I1 -Q1 -Gtt.nc
grdimage tt.nc -J -B30g30:."-Q1": -Ctt.cpt -X4.9i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
sphinterpolate mars370.txt -Rg -I1 -Q2 -Gtt.nc
grdimage tt.nc -J -B30g30:."-Q2": -Ctt.cpt -X-4.9i -Y-5i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
sphinterpolate mars370.txt -Rg -I1 -Q3 -Gtt.nc
grdimage tt.nc -J -B30g30:."-Q3": -Ctt.cpt -X4.9i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
psxy -Rg -J -O -K mars370.txt -Sc0.05i -G0 -B30g30 -X-2.45i -Y2.5i >> $ps
psxy -Rg -J -O -T >> $ps

pscmp
