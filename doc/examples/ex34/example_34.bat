REM             GMT EXAMPLE 34
REM
REM Purpose:    Illustrate pscoast with DCW country polygons
REM GMT progs:  pscoast, makecpt, grdimage
REM DOS calls:  del
REM

echo GMT EXAMPLE 34
set ps=example_34.ps

REM Extract a subset of ETOPO2m for this part of Europe
REM gmt grdcut etopo2m_grd.nc -R -GFR+IT.nc=ns
gmt set FORMAT_GEO_MAP dddF
gmt pscoast -JM4.5i -R-6/20/35/52 -EFR,IT+gP300/8 -Glightgray -Baf -BWSne -P -K -X2i > %ps%
gmt makecpt -Cglobe -T-5000/5000 > z.cpt
gmt grdimage @FR+IT.nc -I+a15+ne0.75 -Cz.cpt -J -O -K -Y4.5i -Baf -BWsnE+t"Franco-Italian Union, 2042-45" >> %ps%
gmt pscoast -J -R -EFR,IT+gred@60 -O >> %ps%
REM cleanup
del gmt.conf z.cpt
