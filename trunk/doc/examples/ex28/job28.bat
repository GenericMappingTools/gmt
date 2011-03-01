REM		GMT EXAMPLE 28
REM		$Id: job28.bat,v 1.5 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrates how to mix UTM data and UTM projection
REM GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast, pstext, mapproject
REM DOS calls:	del, echo, grep, $AWK
REM

echo GMT EXAMPLE 28
set master=y
if exist job28.bat set master=n
if %master%==y cd ex28
	
REM Get intensity grid and set up a color table
grdgradient Kilauea.utm.nc -Nt1 -A45 -GKilauea.utm_i.nc
makecpt -Ccopper -T0/1500/100 -Z > Kilauea.cpt
REM Save min/max UTM coordinates with enough precision
grdinfo Kilauea.utm.nc --D_FORMAT=%%.10g -C > tmp.txt
REM Use inverse UTM projection to determine the lon/lat of the lower left and upper right corners
REM LL=`cut -f2,4 tmp.txt | mapproject -Ju5Q/1:1 -F -C -I --OUTPUT_DEGREE_FORMAT=ddd:mm:ss.x | gawk '{printf "%s/%s\n", $1, $2}'`
REM UR=`cut -f3,5 tmp.txt | mapproject -Ju5Q/1:1 -F -C -I --OUTPUT_DEGREE_FORMAT=ddd:mm:ss.x | gawk '{printf "%s/%s\n", $1, $2}'`
set LL=-155:20:13.3/19:14:51.5
set UR=-155:05:44.2/19:27:54.6
REM Lay down the UTM topo grid using a 1:17,000 scale
grdimage Kilauea.utm.nc -IKilauea.utm_i.nc -CKilauea.cpt -Jx1:170000 -P -K -B5000g5000WSne -U"Example 28 in Cookbook" --D_FORMAT=%%.10g --ANNOT_FONT_SIZE_PRIMARY=9 --GRID_CROSS_SIZE_PRIMARY=0.1i > ..\example_28.ps
REM Overlay geographic data and coregister by using correct region and projection with the same scale
pscoast -R%LL%/%UR%r -Ju5Q/1:170000 -O -K -Df+ -Slightblue -W0.5p -B5mg5mNE --PLOT_DEGREE_FORMAT=ddd:mmF --ANNOT_FONT_SIZE_PRIMARY=12 >> ..\example_28.ps
psbasemap -R -J -O -K -Lf155:07:30W/19:15:40N/19:23N/5k+l1:17,000+u --ANNOT_FONT_SIZE_PRIMARY=10 --LABEL_FONT_SIZE=12 >> ..\example_28.ps
echo 155:16:20W 19:26:20N 12 0 1 CB KILAUEA | pstext -R -J -O >> ..\example_28.ps
REM Clean up

del Kilauea.utm_i.nc
del Kilauea.cpt
del tmp.txt
del .gmt*
if %master%==y cd ..
