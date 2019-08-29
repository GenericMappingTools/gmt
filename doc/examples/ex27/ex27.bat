REM
REM		GMT EXAMPLE 27
REM
REM Purpose:	Illustrates how to plot Mercator img grids
REM GMT progs:	makecpt, grdimage, pscoast
REM GMT supplement: img2grd (to read Sandwell/Smith img files)
REM DOS calls:	del
REM
echo GMT EXAMPLE 27
set ps=example_27.ps

REM Gravity in tasman_grav.nc is in 0.1 mGal increments and the grid
REM is already in projected Mercator x/y units.

REM Make a suitable cpt file for mGal

gmt makecpt -T-120/120 -Crainbow > grav.cpt

REM Since this is a Mercator grid we use a linear gmt projection

gmt grdimage @tasman_grav.nc=ns+s0.1 -I+a45+nt1 -Jx0.25i -Cgrav.cpt -P -K > %ps%

REM Then use gmt pscoast to plot land; get original -R from grid info
REM and use Mercator gmt projection with same scale as above on a spherical Earth

gmt pscoast -R145/170/-50.0163575733/-24.9698584055 -Jm0.25i -Ba10f5 -BWSne -O -K -Gblack --PROJ_ELLIPSOID=Sphere -Cwhite -Dh+ --FORMAT_GEO_MAP=dddF >> %ps%

REM Put a color legend in top-left corner of the land mask

gmt psscale -DjTL+o1c+w2i/0.15i -R -J -Cgrav.cpt -Bx50f10 -By+lmGal  -F+gwhite+p1p -I -O >> %ps%

REM Clean up

del grav.cpt
del .gmt*
