REM		GMT EXAMPLE 04
REM
REM		$Id: job04c.bat,v 1.5 2011-06-23 17:47:56 remko Exp $
REM
REM 3-D perspective color plot of Hawaiian topography and geoid
REM GMT progs:	grdcontour, grdgradient, grdimage, grdview, psbasemap, pscoast, pstext
REM DOS calls:	echo, del
REM
REM CPS: topo.cpt
REM CPS: geoid.cpt
echo GMT EXAMPLE 4 (color)
set ps=..\example_04c.ps
grdgradient HI_geoid4.nc -A0 -Gg_intens.nc -Nt0.75 -fg
grdgradient HI_topo4.nc -A0 -Gt_intens.nc -Nt0.75 -fg
REM
grdimage HI_geoid4.nc -Ig_intens.nc -R195/210/18/25 -JM6.75i -p60/30 -Cgeoid.cpt -E100 -K -X1.5i -Y1.25i -P -U/-1.25i/-1i/"Example 04c in Cookbook" > %ps%
pscoast -R -J -p -B2/2NEsw -Gblack -O -K >> %ps%
psscale -R -J -p -D3.375i/4.3i/5i/0.3ih -Cgeoid.cpt -I -O -K -A "-B2:Geoid (m):" >> %ps%
psbasemap -R -J -p -O -K -T209/19.5/1i --COLOR_BACKGROUND=red --MAP_TICK_PEN_PRIMARY=thinner,red --FONT=red >> %ps%
grdview HI_topo4.nc -It_intens.nc -R195/210/18/25/-6/4 -J -JZ3.4i -p -Ctopo.cpt -N-6/lightgray -Qc100 -O -K -Y2.2i -B2/2/2:"Topo (km)":neswZ >> %ps%
echo 3.25 5.75 H@#awaiian@# R@#idge | pstext -R0/10/0/10 -Jx1i -F+f60p,ZapfChancery-MediumItalic+jCB -O >> %ps%
del *_intens.nc
del .gmt*
