REM		GMT EXAMPLE 28
REM
REM Purpose:	Illustrates how to mix UTM data and UTM projection
REM GMT progs:	makecpt, grdimage, pscoast, pstext
REM DOS calls:	del, echo
REM

echo GMT EXAMPLE 28
set ps=example_28.ps
	
REM Get intensity grid and set up a color table
gmt makecpt -Ccopper -T0/1500 > Kilauea.cpt

REM Lay down the UTM topo grid using a 1:160,000 scale
gmt grdimage @Kilauea.utm.nc -I+a45+nt1 -CKilauea.cpt -Jx1:160000 -P -K --FONT_ANNOT_PRIMARY=9p --MAP_GRID_CROSS_SIZE_PRIMARY=0.1i > %ps%

REM Overlay geographic data and coregister by using correct region and projection with the same scale
gmt pscoast -R@Kilauea.utm.nc -Ju5Q/1:160000 -O -K -Df+ -Slightblue -W0.5p -B5mg5m -BNE --FORMAT_GEO_MAP=ddd:mmF --FONT_ANNOT_PRIMARY=12p >> %ps%
echo 155:16:20W 19:26:20N KILAUEA | gmt pstext -R -J -O -K -F+f12p,Helvetica-Bold+jCB >> %ps%
gmt psbasemap -R -J -O -K -LjRB+c19:23N+f+w5k+l1:160,000+u+o0.2i --FONT_ANNOT_PRIMARY=10p --FONT_LABEL=10p >> %ps%

REM Annotate in km but append 000m to annotations to get customized meter labels
gmt psbasemap -R@Kilauea.utm.nc+Uk -Jx1:160 -B5g5+u"@:8:000m@::" -BWSne -O --FONT_ANNOT_PRIMARY=10p --MAP_GRID_CROSS_SIZE_PRIMARY=0.1i --FONT_LABEL=10p >> %ps%

REM Clean up

del Kilauea.cpt
del .gmt*
