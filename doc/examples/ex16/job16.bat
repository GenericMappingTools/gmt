REM		GMT EXAMPLE 16
REM
REM		$Id: job16.bat,v 1.15 2011-06-09 16:41:18 remko Exp $
REM
REM Purpose:	Illustrates interpolation methods using same data as Example 12.
REM GMT progs:	gmtset, grdview, grdfilter, pscontour, psscale, pstext, surface, triangulate
REM DOS calls:	echo, del
REM
REM First make a cpt file as in example 12:
REM
echo GMT EXAMPLE 16
set ps=..\example_16.ps
REM
REM Illustrate various means of contouring, using triangulate and surface.
REM
gmtset FONT_ANNOT_PRIMARY 9p
REM
pscontour -R0/6.5/-0.2/6.5 -Jx0.45i -P -K -Y5.5i -Ba2f1WSne table_5.11 -Cex16.cpt -I > %ps%
echo 3.25 7 pscontour (triangulate) | pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> %ps%
REM
surface table_5.11 -R -I0.1 -Graws0.nc
grdview raws0.nc -R -J -B -Cex16.cpt -Qs -O -K -X3.5i >> %ps%
echo 3.25 7 surface (tension = 0) | pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> %ps%
REM
surface table_5.11 -R -I0.1 -Graws5.nc -T0.5
grdview raws5.nc -R -J -B -Cex16.cpt -Qs -O -K -Y-3.75i -X-3.5i >> %ps%
echo 3.25 7 surface (tension = 0.5) | pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> %ps%
REM
triangulate table_5.11 -Grawt.nc -R -I0.1 > NUL
grdfilter rawt.nc -Gfiltered.nc -D0 -Fc1
grdview filtered.nc -R -J -B -Cex16.cpt -Qs -O -K -X3.5i >> %ps%
echo 3.25 7 triangulate @~\256@~ grdfilter | pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> %ps%
echo 3.2125 7.5 Gridding of Data | pstext -R0/10/0/10 -Jx1i -O -K -N -F+f32p,Times-Roman+jCB -X-3.5i >> %ps%
psscale -D3.21/0.35/5/0.25h -Cex16.cpt -O -U"Example 16 in Cookbook" -Y-0.75i >> %ps%
del *.nc
del .gmt*
