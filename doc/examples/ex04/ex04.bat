REM		GMT EXAMPLE 04
REM
REM Purpose:	3-D mesh and color plot of Hawaiian topography and geoid
REM GMT modules:	grdcontour, grdimage, grdview, basemap, coast, text, makecpt
REM DOS calls:	echo
REM
gmt begin
	gmt figure ex04
	gmt makecpt -C255,100 -T-10/10/10 -N
	gmt grdcontour @HI_geoid_04.nc -R195/210/18/25 -Jm1c -p60/30 -C1 -A5+o -Gd10c
	gmt coast -p -B2 -BNEsw -Gblack -TdjBR+o0.3c+w3c+l
	gmt grdview @HI_topo_04.nc -R195/210/18/25/-6/4 -Jz0.8c -p -C -N-6+glightgray -Qsm -B2 -Bz2+l"Topo (km)" -BneswZ+t"H@#awaiian@# R@#idge@#" -Y5c --FONT_TITLE=50p,ZapfChancery-MediumItalic --MAP_TITLE_OFFSET=-4c

	gmt figure ex04c
	gmt grdimage @HI_geoid_04.nc -I+a0+nt0.75 -R195/210/18/25 -JM15c -p60/30 -C@geoid_04.cpt -X1.25i -Y1.25i
	gmt coast -p -B2 -BNEsw -Gblack
	gmt basemap -p -TdjBR+o0.3c+w3c+l --COLOR_BACKGROUND=red --FONT=red --MAP_TICK_PEN_PRIMARY=thinner,red
	gmt colorbar -p240/30 -DJBC+o0/1c+w13c/0.75c+h -C@geoid_04.cpt -I -Bx2+l"Geoid (m)"
	gmt grdview @HI_topo_04.nc -I+a0+nt0.75 -R195/210/18/25/-6/4 -JZ8c -p60/30 -C@topo_04.cpt -N-6+glightgray -Qc100 -B2 -Bz2+l"Topo (km)" -BneswZ+t"H@#awaiian@# R@#idge@#" -Y5c --FONT_TITLE=50p,ZapfChancery-MediumItalic --MAP_TITLE_OFFSET=-4c
gmt end show
