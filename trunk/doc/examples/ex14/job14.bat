REM		GMT EXAMPLE 14
REM
REM		$Id: job14.bat,v 1.17 2011-05-18 16:24:14 remko Exp $
REM
REM Purpose:	Showing simple gridding, contouring, and resampling along tracks
REM GMT progs:	gmtset, blockmean, grdcontour, grdtrack, grdtrend, minmax, project, pstext
REM GMT progs:	psbasemap, psxy, surface
REM DOS calls:	del
REM
REM First draw network and label the nodes
echo GMT EXAMPLE 14
set ps=..\example_14.ps
gmtset MAP_GRID_PEN_PRIMARY thinnest,-
psxy table_5.11 -R0/7/0/7 -JX3.06i/3.15i -B2f1WSNe -Sc0.05i -Gblack -P -K -Y6.45i > %ps%
pstext table_5.11 -R -J -D0.1c/0 -F+f6p+jLM -O -K -N >> %ps%
blockmean table_5.11 -R0/7/0/7 -I1 > mean.xyz
REM Then draw blocmean cells
psbasemap -R0.5/7.5/0.5/7.5 -J -O -K -Bg1 -X3.25i >> %ps%
psxy -R0/7/0/7 -J -B2f1eSNw mean.xyz -Ss0.05i -Gblack -O -K >> %ps%
gmtconvert mean.xyz --FORMAT_FLOAT_OUT=%%.1f | pstext -R -J -D0.15c/0 -F+f6p+jLM -O -K -Gwhite -W -C0.01i -N >> %ps%
REM Then surface and contour the data
surface mean.xyz -R -I1 -Gdata.nc
grdcontour data.nc -J -B2f1WSne -C25 -A50 -Gd3i -S4 -O -K -X-3.25i -Y-3.55i >> %ps%
psxy -R -J mean.xyz -Ss0.05i -Gblack -O -K >> %ps%
REM Fit bicubic trend to data and compare to gridded surface
grdtrend data.nc -N10 -Ttrend.nc
project -C0/0 -E7/7 -G0.1 -N > track
grdcontour trend.nc -J -B2f1wSne -C25 -A50 -Glct/cb -S4 -O -K -X3.25i >> %ps%
psxy -R -J track -Wthick,. -O -K >> %ps%
REM Sample along diagonal
grdtrack track -Gdata.nc -o2,3 > data.d
grdtrack track -Gtrend.nc -o2,3 > trend.d
REM minmax data.d trend.d -I0.5/25
REM Use result of minmax manually in -R below:
psxy -R0/10/775/1050 -JX6.3i/1.4i data.d -Wthick -O -K -X-3.25i -Y-1.9i -B1/50WSne >> %ps%
psxy -R -J trend.d -Wthinner,- -O -U"Example 14 in Cookbook" >> %ps%
del mean.xyz
del track
del *.nc
del *.d
del .gmt*
