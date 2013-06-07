REM		GMT EXAMPLE 28
REM		$Id$
REM
REM Purpose:	Illustrates how to mix UTM data and UTM projection
REM GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast, pstext
REM DOS calls:	del, echo
REM

echo GMT EXAMPLE 28
set ps=example_28.ps
	
REM Get intensity grid and set up a color table
gmt grdgradient Kilauea.utm.nc -Nt1 -A45 -GKilauea.utm_i.nc
gmt makecpt -Ccopper -T0/1500/100 -Z > Kilauea.cpt

REM Lay down the UTM topo grid using a 1:16,000 scale
gmt grdimage Kilauea.utm.nc -IKilauea.utm_i.nc -CKilauea.cpt -Jx1:160000 -P -K --FORMAT_FLOAT_OUT=%%.10g --FONT_ANNOT_PRIMARY=9p --MAP_GRID_CROSS_SIZE_PRIMARY=0.1i > %ps%

REM Overlay geographic data and coregister by using correct region and projection with the same scale
gmt pscoast -RKilauea.utm.nc -Ju5Q/1:160000 -O -K -Df+ -Slightblue -W0.5p -B5mg5m -BNE --FORMAT_GEO_MAP=ddd:mmF --FONT_ANNOT_PRIMARY=12p >> %ps%
echo 155:16:20W 19:26:20N KILAUEA | gmt pstext -R -J -O -K -F+f12p,Helvetica-Bold+jCB >> %ps%
gmt psbasemap -R -J -O -K -Lf155:07:30W/19:15:40N/19:23N/5k+l1:16,000+u --FONT_ANNOT_PRIMARY=10p --FONT_LABEL=10p >> %ps%

REM Annotate in km but append 000m to annotations to get customized meter labels
gmt psbasemap -RKilauea.utm.nc+Uk -Jx1:160 -B5g5+u"@:8:000m" -BWSne -O --FONT_ANNOT_PRIMARY=10p --MAP_GRID_CROSS_SIZE_PRIMARY=0.1i --FONT_LABEL=10p >> %ps%

REM Clean up

del Kilauea.utm_i.nc
del Kilauea.cpt
del .gmt*
