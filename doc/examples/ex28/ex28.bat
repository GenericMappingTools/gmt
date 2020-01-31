REM		GMT EXAMPLE 28
REM
REM Purpose:	Illustrates how to mix UTM data and UTM gmt projection
REM GMT modules:	makecpt, grdimage, coast, text, basemap
REM
gmt begin ex28
	REM Set up a color table
	gmt makecpt -Ccopper -T0/1500
	REM Lay down the UTM topo grid using a 1:160,000 scale
	gmt grdimage @Kilauea.utm.nc -I+d -Jx1:160000 --FONT_ANNOT_PRIMARY=9p
	REM Overlay geographic data and coregister by using correct region and gmt projection with the same scale
	gmt coast -R@Kilauea.utm.nc -Ju5Q/1:160000 -Df+ -Slightblue -W0.5p -B5mg5m -BNE --FONT_ANNOT_PRIMARY=12p --FORMAT_GEO_MAP=ddd:mmF
	echo 155:16:20W 19:26:20N KILAUEA | gmt text -F+f12p,Helvetica-Bold+jCB
	gmt basemap --FONT_ANNOT_PRIMARY=9p -LjRB+c19:23N+f+w5k+l1:160,000+u+o0.5c --FONT_LABEL=10p
	REM Annotate in km but append ,000m to annotations to get customized meter labels
	gmt basemap -R@Kilauea.utm.nc+Uk -Jx1:160 -B5g5+u"@:8:000m@::" -BWSne --FONT_ANNOT_PRIMARY=10p --MAP_GRID_CROSS_SIZE_PRIMARY=0.25c --FONT_LABEL=10p
gmt end show
