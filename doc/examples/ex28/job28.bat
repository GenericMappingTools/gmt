REM		GMT EXAMPLE 28
REM		$Id: job28.bat,v 1.7 2011-04-23 22:06:48 jluis Exp $
REM
REM Purpose:	Illustrates how to mix UTM data and UTM projection
REM GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast, pstext, mapproject
REM DOS calls:	del, echo, gawk
REM

echo GMT EXAMPLE 28
set ps=..\example_28.ps
	
REM Get intensity grid and set up a color table
grdgradient Kilauea.utm.nc -Nt1 -A45 -GKilauea.utm_i.nc
makecpt -Ccopper -T0/1500/100 -Z > Kilauea.cpt

REM Save min/max UTM coordinates with enough precision
grdinfo Kilauea.utm.nc --FORMAT_FLOAT_OUT=%%.10g -C > tmp.txt

REM Use inverse UTM projection to determine the lon/lat of the lower left and upper right corners
REM LL=`mapproject tmp.txt -Ju5Q/1:1 -F -C -I --FORMAT_GEO_OUT=ddd:mm:ss.x -i1,3 -o0,1 | gawk '{printf "%s/%s\n", $1, $2}'`
REM UR=`mapproject tmp.txt -Ju5Q/1:1 -F -C -I --FORMAT_GEO_OUT=ddd:mm:ss.x -i2,4 -o0,1 | gawk '{printf "%s/%s\n", $1, $2}'`
set LL=-155:20:13.3/19:14:51.5
set UR=-155:05:44.2/19:27:54.6

REM Lay down the UTM topo grid using a 1:16,000 scale
grdimage Kilauea.utm.nc -IKilauea.utm_i.nc -CKilauea.cpt -Jx1:160000 -P -K -U"Example 28 in Cookbook" --FORMAT_FLOAT_OUT=%%.10g --FONT_ANNOT_PRIMARY=9p --MAP_GRID_CROSS_SIZE_PRIMARY=0.1i > %ps%

REM Overlay geographic data and coregister by using correct region and projection with the same scale
pscoast -R%LL%/%UR%r -Ju5Q/1:160000 -O -K -Df+ -Slightblue -W0.5p -B5mg5mNE --FORMAT_GEO_MAP=ddd:mmF --FONT_ANNOT_PRIMARY=12p >> %ps%
echo 155:16:20W 19:26:20N KILAUEA | pstext -R -J -O -K -F+f12p,Helvetica-Bold+jCB >> %ps%
psbasemap -R -J -O -K -Lf155:07:30W/19:15:40N/19:23N/5k+l1:16,000+u --FONT_ANNOT_PRIMARY=10p --FONT_LABEL=10p >> %ps%

REM Annotate in km but append 000m to annotations to get customized meter labels 
psbasemap -R254.34/280.02/2129.88/2153.64 -Jx1:160 -B5g5:,"-@:8:000m":WSne -O --FONT_ANNOT_PRIMARY=10p --MAP_GRID_CROSS_SIZE_PRIMARY=0.1i --FONT_LABEL=10p >> %ps%

REM Clean up

del Kilauea.utm_i.nc
del Kilauea.cpt
del tmp.txt
del .gmt*
