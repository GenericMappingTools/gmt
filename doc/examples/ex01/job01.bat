REM
REM		GMT EXAMPLE 01
REM
REM		$Id: job01.bat,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
REM
REM Purpose:	Make two contour maps based on the data in the file osu91a1f_16.grd
REM GMT progs:	gmtset grdcontour psbasemap pscoast
REM DOS calls:	del
REM
echo GMT EXAMPLE 01
set master=y
if exist job01.bat set master=n
if %master%==y cd ex01
gmtset GRID_CROSS_SIZE 0 ANOT_FONT_SIZE 10
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -K -U"Example 1 in Cookbook" > example_01.ps
pscoast -R-180/180/-90/90 -JH0/6i -X0.25i -Y0.5i -O -K -Bg30 -Dc -G200 >> example_01.ps
grdcontour -R osu91a1f_16.grd -JH -C10 -A50f7 -G4i -L-1000/-1 -Wc0.25pta -Wa0.75pt2_2:0 -O -K -T0.1i/0.02i >> example_01.ps
grdcontour -R osu91a1f_16.grd -JH -C10 -A50f7 -G4i -L-1/1000 -O -K -T0.1i/0.02i >> example_01.ps
pscoast -R0/360/-90/90 -JH180/6i -Y4i -O -K -Bg30:."Low Order Geoid": -Dc -G200 >> example_01.ps
grdcontour osu91a1f_16.grd -JH -C10 -A50f7 -G4i -L-1000/-1 -Wc0.25pta -Wa0.75pt2_2:0 -O -K -T0.1i/0.02i:-+ >> example_01.ps
grdcontour osu91a1f_16.grd -JH -C10 -A50f7 -G4i -L-1/1000 -O -T0.1i/0.02i:-+ >> example_01.ps
del .gmt*
if %master%==y cd ..
