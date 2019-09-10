REM
REM		GMT EXAMPLE 01
REM
REM
REM Purpose:	Make two contour maps based on the data in the file osu91a1f_16.nc
REM GMT progs:	gmtset, grdcontour, psbasemap, pscoast
REM DOS calls:	del
REM
echo GMT EXAMPLE 01
set ps=example_01.ps
gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0 FONT_ANNOT_PRIMARY 10p
gmt psbasemap -R0/6.5/0/7.5 -Jx1i -B0 -P -K > %ps%
gmt pscoast -Rg -JH0/6i -X0.25i -Y0.2i -O -K -Bg30 -Dc -Glightbrown -Slightblue >> %ps%
gmt grdcontour @osu91a1f_16.nc -J -C10 -A50+f7p -Gd4i -L-1000/-1 -Wcthinnest,- -Wathin,- -O -K -T+d0.1i/0.02i >> %ps%
gmt grdcontour @osu91a1f_16.nc -J -C10 -A50+f7p -Gd4i -L-1/1000 -O -K -T+d0.1i/0.02i >> %ps%
gmt pscoast -Rg -JH6i -Y3.4i -O -K -B+t"Low Order Geoid" -Bg30 -Dc -Glightbrown -Slightblue >> %ps%
gmt grdcontour @osu91a1f_16.nc -J -C10 -A50+f7p -Gd4i -L-1000/-1 -Wcthinnest,- -Wathin,- -O -K -T+d0.1i/0.02i+l >> %ps%
gmt grdcontour @osu91a1f_16.nc -J -C10 -A50+f7p -Gd4i -L-1/1000 -O -T+d0.1i/0.02i+l >> %ps%
del .gmt*
del gmt.conf
