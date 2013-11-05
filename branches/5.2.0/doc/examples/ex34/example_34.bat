REM             GMT EXAMPLE 34
REM             $Id
REM
REM Purpose:    Illustrate pscoast with DCW country polygons
REM GMT progs:  pscoast, makecpt, grdimage, grdgradient
REM DOS calls:  del
REM

echo GMT EXAMPLE 34
set ps=example_34.ps

REM Extract a subset of ETOPO2m for this part of Europe
REM gmt grdcut etopo2m_grd.nc -R -GFR+IT.nc=ns
gmt gmtset FORMAT_GEO_MAP dddF
gmt pscoast -JM4.5i -R-6/20/35/52 -FFR,IT+gP300/8 -Glightgray -Baf -BWSne -P -K -X2i > %ps%
gmt makecpt -Cglobe -T-5000/5000/500 -Z > z.cpt
gmt grdgradient FR+IT.nc -A15 -Ne0.75 -GFR+IT_int.nc
gmt grdimage FR+IT.nc -IFR+IT_int.nc -Cz.cpt -J -O -K -Y4.5i -Baf -BWsnE+t"Franco-Italian Union, 2042-45" >> %ps%
gmt pscoast -J -R -FFR,IT+gred@60 -O >> %ps%
REM cleanup
del gmt.conf FR+IT_int.nc z.cpt
