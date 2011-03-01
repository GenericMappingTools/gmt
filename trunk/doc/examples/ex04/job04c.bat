REM		GMT EXAMPLE 04
REM
REM		$Id: job04c.bat,v 1.3 2011-03-01 01:34:48 remko Exp $
REM
REM 3-D perspective color plot of Hawaiian topography and geoid
REM GMT progs:	grdcontour, grdview, pscoast, pstext
REM DOS calls:	echo, del
REM
REM CPS: topo.cpt
REM CPS: geoid.cpt
echo GMT EXAMPLE 4 (color)
grdgradient HI_geoid4.nc -A0 -Gg_intens.nc -Nt0.75 -M
grdgradient HI_topo4.nc -A0 -Gt_intens.nc -Nt0.75 -M
REM
grdview HI_geoid4.nc -Ig_intens.nc -JM6.75i -E60/30 -R195/210/18/25 -Cgeoid.cpt -Qi100 -K -X1.5i -Y1.25i -P -U/-1.25i/-1i/"Example 04c in Cookbook" > ..\example_04c.ps
pscoast -J -E60/30 -R -B2/2NEsw -Gblack -O -K >> ..\example_04c.ps
psbasemap -R -J -E60/30 -O -K -T209/19.5/1i --COLOR_BACKGROUND=red --TICK_PEN=thinner,red >> ..\example_04c.ps
grdview HI_topo4.nc -It_intens.nc -J -JZ3.4i -Ctopo.cpt -E60/30 -R195/210/18/25/-6/4 -N-6/lightgray -Qi100 -O -K -Y2.2i >> ..\example_04c.ps
psbasemap -J -JZ3.4i -E60/30 -R -Z-6 -O -K -B2/2/2:"Topo (km)":neZ >> ..\example_04c.ps
echo 3.25 5.75 60 0.0 33 2 H@#awaiian@# R@#idge | pstext -R0/10/0/10 -Jx1i -O >> ..\example_04c.ps
del *_intens.nc
del .gmt*
