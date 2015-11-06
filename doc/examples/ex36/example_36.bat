REM             GMT EXAMPLE 36
REM             $Id
REM
REM Purpose:    Illustrate sphinterpolate with Mars radii data
REM GMT progs:  psxy, makecpt, grdimage, sphinterpolate
REM DOS calls:  del
REM

echo GMT EXAMPLE 36
set ps=example_36.ps

REM Interpolate data of Mars radius from Mariner9 and Viking Orbiter spacecrafts
gmt makecpt -Crainbow -T-7000/15000/1000 -Z > tt.cpt
REM Piecewise linear interpolation; no tension
gmt sphinterpolate mars370.txt -Rg -I1 -Q0 -Gtt.nc
gmt grdimage tt.nc -JH0/6i -Bag -Ctt.cpt -P -Xc -Y7.25i -K  > %ps%
gmt psxy -Rg -J -O -K mars370.txt -Sc0.05i -G0 -B30g30 -Y-3.25i >> %ps%
REM Smoothing
gmt sphinterpolate mars370.txt -Rg -I1 -Q3 -Gtt.nc
gmt grdimage tt.nc -J -Bag -Ctt.cpt  -Y-3.25i -O -K >> %ps%
gmt psxy -Rg -J -O -T >> %ps%
REM cleanup
del tt.cpt tt.nc
