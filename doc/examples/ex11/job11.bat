REM		GMT EXAMPLE 11
REM
REM		$Id: job11.bat,v 1.5 2004-04-10 17:19:14 pwessel Exp $
REM
REM Purpose:	Create a 3-D RGB Cube
REM GMT progs:	gmtset, grdimage, grdmath, pstext, psxy
REM DOS calls:	echo, del, gawk
REM
REM First create a Plane from (0,0,0) to (255,255,255).
REM Only needs to be done once, and is used on each of the 6 faces of the cube.
REM
echo GMT EXAMPLE 11
set master=y
if exist job11.bat set master=n
if %master%==y cd ex11

grdmath -I1 -R0/255/0/255 Y 256 MUL X ADD = rgb_cube.grd

REM
REM For each of the 6 faces, create a color palette with one color (r,g,b) fixed
REM at either the min. of 0 or max. of 255, and the other two components
REM varying smoothly across the face from 0 to 255.
REM
REM This uses gawk script "rgb_cube.awk", with arguments specifying which color
REM (r,g,b) is held constant at 0 or 255, which color varies in the x-direction
REM of the face, and which color varies in the y-direction.  If the color is to
REM increase in x (y), a lower case x (y) is indicated; if the color is to 
REM decrease in the x (y) direction, an upper case X (Y) is used.
REM
REM Use grdimage to paint the faces and psxy to add "cut-along-the-dotted" lines.
REM

gmtset TICK_LENGTH 0 COLOR_MODEL rgb

pstext -R0/8/0/11 -Jx1i NUL -P -U"Example 11 in Cookbook" -K > example_11.ps
gawk -f rgb_cube.awk r=x g=y b=255 < NUL > rgb_cube.cpt
grdimage rgb_cube.grd -Crgb_cube.cpt -JX2.5i/2.5i -R0/255/0/255 -K -O -X2i -Y4.5i -B256wesn >> example_11.ps
gawk -f rgb_cube.awk r=255 g=y b=X < NUL > rgb_cube.cpt
grdimage rgb_cube.grd -Crgb_cube.cpt -J -K -O -X2.5i -B256wesn >> example_11.ps

gawk -f rgb_cube.awk r=x g=255 b=Y < NUL > rgb_cube.cpt
grdimage rgb_cube.grd -Crgb_cube.cpt -J -K -O -X-2.5i -Y2.5i -B256wesn >> example_11.ps
echo 0 0 > ex11_1.d
echo 20 20 >> ex11_1.d
echo 20 235 >> ex11_1.d
echo 0 255 >> ex11_1.d
psxy -W0.25pto -J -R -K -O -X2.5i ex11_1.d >> example_11.ps
echo 0 0 > ex11_2.d
echo 20 20 >> ex11_2.d
echo 235 20 >> ex11_2.d
echo 255 0 >> ex11_2.d
psxy -W0.25pto -J -R -K -O -X-2.5i -Y2.5i ex11_2.d >> example_11.ps
echo 255 0 > ex11_3.d
echo 235 20 >> ex11_3.d
echo 235 235 >> ex11_3.d
echo 255 255 >> ex11_3.d
psxy -W0.25pto -J -R -K -O -X-2.5i -Y-2.5i ex11_3.d >> example_11.ps

gawk -f rgb_cube.awk r=0 g=y b=x < NUL > rgb_cube.cpt
grdimage rgb_cube.grd -Crgb_cube.cpt -J -K -O -Y-2.5i -B256wesn >> example_11.ps

gawk -f rgb_cube.awk r=x g=0 b=y < NUL > rgb_cube.cpt
grdimage rgb_cube.grd -Crgb_cube.cpt -J -K -O -X2.5i -Y-2.5i -B256wesn >> example_11.ps

echo 10 10 14 0 Times-BoldItalic BL GMT 4 | pstext -J -R -Gwhite -K -O >> example_11.ps

psxy -W0.25pto -J -R -K -O -X2.5i ex11_1.d >> example_11.ps
psxy -W0.25pto -J -R -K -O -X-5i ex11_3.d >> example_11.ps

gawk -f rgb_cube.awk r=x g=Y b=0 < NUL > rgb_cube.cpt
grdimage rgb_cube.grd -Crgb_cube.cpt -J -K -O -X2.5i -Y-2.5i -B256wesn >> example_11.ps

psxy -W0.25pto -J -R -K -O -X2.5i ex11_1.d >> example_11.ps
psxy -W0.25pto -J -R -O -X-5i ex11_3.d >> example_11.ps

del rgb_cube.cpt
del rgb_cube.grd
del ex11_?.d
del .gmt*
if %master%==y cd ..
