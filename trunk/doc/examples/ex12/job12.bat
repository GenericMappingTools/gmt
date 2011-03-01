REM		GMT EXAMPLE 12
REM
REM		$Id: job12.bat,v 1.11 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrates Delaunay triangulation of points, and contouring
REM GMT progs:	makecpt, minmax, pscontour, pstext, psxy, triangulate
REM DOS:	echo, del, gawk
REM Remark:	Differs from UNIX version in that makecpt uses hardwired limits
REM
REM First draw network and label the nodes
REM
echo GMT EXAMPLE 12
set master=y
if exist job12.bat set master=n
if %master%==y cd ex12
triangulate table_5.11 -m > net.xy
psxy -R0/6.5/-0.2/6.5 -JX3.06i/3.15i -B2f1WSNe -m net.xy -Wthinner -P -K -X0.9i -Y4.65i > ..\example_12.ps
psxy table_5.11 -R -J -O -K -Sc0.12i -Gwhite -Wthinnest >> ..\example_12.ps
gawk "{print $1, $2, 6, 0, 0, 6, NR-1}" table_5.11 | pstext -R -J -O -K >> ..\example_12.ps
REM
REM Then draw network and print the node values
REM
psxy -R -J -B2f1eSNw -m net.xy -Wthinner -O -K -X3.25i >> ..\example_12.ps
psxy -R -J -O -K table_5.11 -Sc0.03i -Gblack >> ..\example_12.ps
gawk "{print $1, $2, 6, 0, 0, 5, $3}" table_5.11 | pstext -R -J -O -K -Wwhite,o -C0.01i/0.01i -D0.08i/0i -N >> ..\example_12.ps
REM
REM Then contour the data and draw triangles using dashed pen; use "minmax" and "makecpt" to make a color palette (.cpt) file
REM
makecpt -Cjet -T675/975/25 > topo.cpt
pscontour -R -J table_5.11 -B2f1WSne -Wthin -Ctopo.cpt -Lthinnest,- -G1i -X-3.25i -Y-3.65i -O -K -U"Example 12 in Cookbook" >> ..\example_12.ps
REM
REM Finally color the topography
REM
pscontour -R -J table_5.11 -B2f1eSnw -Ctopo.cpt -I -X3.25i -O -K >> ..\example_12.ps
echo 3.16 8 30 0 1 2 Delaunay Triangulation | pstext -R0/8/0/11 -Jx1i -O -X-3.25i >> ..\example_12.ps
REM
del net.xy
del topo.cpt
del .gmt*
if %master%==y cd ..
