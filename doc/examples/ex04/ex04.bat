REM		GMT EXAMPLE 04
REM
REM Purpose:	3-D mesh and color plot of Hawaiian topography and geoid
REM GMT modules:	grdcontour, grdimage, grdview, basemap, coast, text, makecpt
REM DOS calls:	echo
REM
gmt begin
	gmt figure ex04
	gmt makecpt -C255,100 -T-10/10/10 -N
	gmt grdcontour @HI_geoid_04.nc -R195/210/18/25 -Jm0.45i -p60/30 -C1 -A5+o -Gd4i -X1.25i -Y1.25i
	gmt coast -p -B2 -BNEsw -Gblack -TdjBR+o0.1i+w1i+l
	gmt grdview @HI_topo_04.nc -R195/210/18/25/-6/4 -Jz0.34i -p -C -N-6+glightgray -Qsm -B2 -Bz2+l"Topo (km)" -BneswZ -Y2.2i
	echo 3.25 5.75 H@#awaiian@# R@#idge@# | gmt text -R0/10/0/10 -Jx1i -F+f60p,ZapfChancery-MediumItalic+jCB

	gmt figure ex04c
	gmt grdimage @HI_geoid_04.nc -I+a0+nt0.75 -R195/210/18/25 -JM6.75i -p60/30 -C@geoid_04.cpt -X1.25i -Y1.25i
	gmt coast -p -B2 -BNEsw -Gblack
	gmt basemap -p -TdjBR+o0.1i+w1i+l --COLOR_BACKGROUND=red --FONT=red --MAP_TICK_PEN_PRIMARY=thinner,red
	gmt colorbar -p240/30 -DJBC+o0/0.5i+w5i/0.3i+h -C@geoid_04.cpt -I -Bx2+l"Geoid (m)"
	gmt grdview @HI_topo_04.nc -I+a0+nt0.75 -R195/210/18/25/-6/4 -JZ3.4i -p60/30 -C@topo_04.cpt -N-6+glightgray -Qc100 -B2 -Bz2+l"Topo (km)" -BneswZ -Y2.2i
	echo 3.25 5.75 H@#awaiian@# R@#idge@# | gmt text -R0/10/0/10 -Jx1i -F+f60p,ZapfChancery-MediumItalic+jCB
gmt end show
