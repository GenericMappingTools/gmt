REM		GMT EXAMPLE 04
REM
REM
REM Purpose:	3-D mesh plot of Hawaiian topography and geoid
REM GMT progs:	grdcontour, grdimage, grdview, psbasemap, pscoast, pstext
REM DOS calls:	echo, del
REM
echo GMT EXAMPLE 04
set ps=example_04.ps
gmt makecpt -C255,100 -T-10/10/10 -N > zero.cpt
gmt grdcontour @HI_geoid_04.nc -R195/210/18/25 -Jm0.45i -p60/30 -C1 -A5+o -Gd4i -K -P -X1.5i -Y1.5i > %ps%
gmt pscoast -R -J -p -B2 -BNEsw -Gblack -O -K -TdjBR+o0.1i+w1i+l >> %ps%
gmt grdview @HI_topo_04.nc -R195/210/18/25/-6/4 -J -Jz0.34i -p -Czero.cpt -N-6+glightgray -Qsm -O -K -B2 -Bz2+l"Topo (km)" -BneswZ -Y2.2i >> %ps%
echo 3.25 5.75 H@#awaiian@# R@#idge@# | gmt pstext -R0/10/0/10 -Jx1i -F+f60p,ZapfChancery-MediumItalic+jCB -O >> %ps%
del zero.cpt
REM
echo GMT EXAMPLE 4 (color)
set ps=example_04c.ps
REM
gmt grdimage @HI_geoid_04.nc -I+a0+nt0.75 -R195/210/18/25 -JM6.75i -p60/30 -C@geoid_04.cpt -E100 -K -X1.5i -Y1.25i -P -UL/-1.25i/-1i/"Example 04c in Cookbook" > %ps%
gmt pscoast -R -J -p -B2 -BNEsw -Gblack -O -K >> %ps%
gmt psscale -R -J -p -DjBC+o0/0.5i+jTC+w5i/0.3i+h -C@geoid_04.cpt -I -O -K -Bx2+l"Geoid (m)" >> %ps%
gmt psbasemap -R -J -p -O -K -TdjBR+o0.1i+w1i+l --COLOR_BACKGROUND=red --MAP_TICK_PEN_PRIMARY=thinner,red --FONT=red >> %ps%
gmt grdview @HI_topo_04.nc -I+a0+nt0.75 -R195/210/18/25/-6/4 -J -JZ3.4i -p -C@topo_04.cpt -N-6+glightgray -Qc100 -O -K -Y2.2i -B2 -Bz2+l"Topo (km)" -BneswZ >> %ps%
echo 3.25 5.75 H@#awaiian@# R@#idge@# | gmt pstext -R0/10/0/10 -Jx1i -F+f60p,ZapfChancery-MediumItalic+jCB -O >> %ps%
del .gmt*
