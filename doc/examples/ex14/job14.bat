REM		GMT EXAMPLE 14
REM
REM		$Id: job14.bat,v 1.6 2004-04-12 21:41:47 pwessel Exp $
REM
REM Purpose:	Showing simple gridding, contouring, and resampling along tracks
REM GMT progs:	blockmean, grdcontour, grdtrack, grdtrend, minmax, project
REM		pstext, psbasemap, psxy, surface
REM DOS calls:	del, gawk
REM
REM First draw network and label the nodes
echo GMT EXAMPLE 14
set master=y
if exist job14.bat set master=n
if %master%==y cd ex14
gmtset GRID_PEN_PRIMARY 0.25p,-
psxy table_5.11 -R0/7/0/7 -JX3.06i/3.15i -B2f1WSNe -Sc0.05i -Gblack -P -K -Y6.45i > example_14.ps
gawk "{print $1+0.08, $2, 6, 0, 0, 5, $3}" table_5.11 | pstext -R -J -O -K -N >> example_14.ps
blockmean table_5.11 -R0/7/0/7 -I1 > mean.xyz
REM Then draw blockmean cells
psbasemap -R0.5/7.5/0.5/7.5 -J -O -K -B0g1 -X3.25i >> example_14.ps
psxy -R0/7/0/7 -J -B2f1eSNw mean.xyz -Ss0.05i -Gblack -O -K >> example_14.ps
gawk "{print $1+0.1, $2, 6, 0, 0, 5, $3}" mean.xyz | pstext -R -J -O -K -W255o -C0.01i/0.01i -N >> example_14.ps
REM Then surface and contour the data
surface mean.xyz -R -I1 -Gdata.grd
grdcontour data.grd -J -B2f1WSne -C25 -A50 -G3i/10 -S4 -O -K -X-3.25i -Y-3.55i >> example_14.ps
psxy -R -J mean.xyz -Ss0.05i -Gblack -O -K >> example_14.ps
REM Fit bicubic trend to data and compare to gridded surface
grdtrend data.grd -N10 -Ttrend.grd
grdcontour trend.grd -J -B2f1wSne -C25 -A50 -G3i/10 -S4 -O -K -X3.25i >> example_14.ps
project -C0/0 -E7/7 -G0.1 > track
psxy -R -J track -W1pto -O -K >> example_14.ps
REM Sample along diagonal
grdtrack track -Gdata.grd | gawk "{print $3, $4}" > data.d
grdtrack track -Gtrend.grd | gawk "{print $3, $4}" > trend.d
REM minmax data.d trend.d -I0.5/25
REM Use result of minmax manually in -R below:
psxy -R0/10/775/1050 -JX6.3i/1.4i data.d -W1p -O -K -X-3.25i -Y-1.9i -B1/50WSne >> example_14.ps
psxy -R -J trend.d -W0.5pta -O -U"Example 14 in Cookbook" >> example_14.ps
del mean.xyz
del track
del *.grd
del *.d
del .gmt*
if %master%==y cd ..
