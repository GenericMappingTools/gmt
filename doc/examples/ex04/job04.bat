REM		GMT EXAMPLE 04
REM
REM		$Id: job04.bat,v 1.11 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	3-D mesh plot of Hawaiian topography and geoid
REM GMT progs:	grdcontour, grdview, pscoast, pstext
REM DOS calls:	echo, del
REM
REM CPS: topo.cpt
REM CPS: geoid.cpt
echo GMT EXAMPLE 04
set master=y
if exist job04.bat set master=n
if %master%==y cd ex04
echo -10     255     0       255 > zero.cpt
echo 0       100     10      100 >> zero.cpt
grdcontour HI_geoid4.nc -Jm0.45i -E60/30 -R195/210/18/25 -C1 -A5 -G4i -K -P -X1.5i -Y1.5i -U/-1.25i/-1.25i/"Example 4 in Cookbook" > ..\example_04.ps
pscoast -J -E60/30 -R -B2/2NEsw -Gblack -O -K -T209/19.5/1i >> ..\example_04.ps
grdview HI_topo4.nc -J -Jz0.34i -Czero.cpt -E60/30 -R195/210/18/25/-6/4 -N-6/200/200/200 -Qsm -O -K -B2/2/2:"Topo (km)":neswZ -Y2.2i >> ..\example_04.ps
echo 3.25 5.75 60 0.0 33 2 H@#awaiian@# R@#idge | pstext -R0/10/0/10 -Jx1i -O >> ..\example_04.ps
del zero.cpt
del .gmt*
if %master%==y cd ..
