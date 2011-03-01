REM             GMT EXAMPLE 30
REM             $Id: job30.bat,v 1.5 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Show graph mode and math angles
REM GMT progs:	gmtmath, psbasemap, pstext and psxy
REM DOS calls:	del, echo
REM

echo GMT EXAMPLE 30
set master=y
if exist job30.bat set master=n
if %master%==y cd ex30

set ps=..\example_30.ps

gmtset CHAR_ENCODING Standard+  

psbasemap -R0/360/-1.25/1.75 -JX8i/6i -B90f30:,-\312:/1g10:."Two Trigonometric Functions":WS -K -U"Example 30 in Cookbook" --BASEMAP_TYPE=graph --VECTOR_SHAPE=0.5 > %ps%

REM Draw sine an cosine curves

gmtmath -T0/360/0.1 T COSD = | psxy -R -J -O -K -W2p >> %ps%
gmtmath -T0/360/0.1 T SIND = | psxy -R -J -O -K -W2p,. --PS_LINE_CAP=round >> %ps%

REM Indicate the x-angle = 120 degrees
echo 120 -1.25 > tmp
echo 120 1.25 >> tmp
psxy -R -J -O -K -W0.5p,- tmp >> %ps%

echo 360 1 18 0 4 RB x = cos(@%%12%%a@%%%%) > tmp
echo 360 0 18 0 4 RB y = sin(@%%12%%a@%%%%) >> tmp
echo 120 -1.25 14 0 4 LB 120\312 >> tmp
echo 370 -1.35 24 0 12 LT a >> tmp
echo -5 1.85 24 0 4 RT x,y >> tmp
pstext -R -J -O -K -Dj0.05i -N tmp >> %ps%

REM Draw a circle and indicate the 0-70 degree angle

echo 0 0 | psxy -R-1/1/-1/1 -Jx1.5i -O -K -X3.625i -Y2.75i -Sc2i -W1p -N >> %ps%
echo -1	0 > tmp
echo 1	0 >> tmp
psxy -R -J -O -K -m -W0.25p tmp >> %ps%
echo 0	-1 > tmp
echo 0	1 >> tmp
psxy -R -J -O -K -m -W0.25p tmp >> %ps%
echo 0	0 > tmp
echo 1	0 >> tmp
psxy -R -J -O -K -m -W1p tmp >> %ps%
echo 0	0 > tmp
echo -0.5	0.866025 >> tmp
psxy -R -J -O -K -m -W1p tmp >> %ps%
echo -0.3333	0 > tmp
echo 0	0 >> tmp
psxy -R -J -O -K -m -W2p tmp >> %ps%
echo -0.3333	0.57735 > tmp
echo -0.3333	0 >> tmp
psxy -R -J -O -K -m -W2p tmp >> %ps%

echo -0.16666 0 12 0 4 CT x > tmp
echo -0.3333 0.2888675 12 0 4 RM y >> tmp
echo 0.22 0.27 12 -30 12 CB a >> tmp
echo -0.33333 0.6 12 30 4 LB 120\312 >> tmp
pstext -R -J -O -K -Dj0.05i tmp >> %ps%

echo 0 0 0 120 | psxy -R -J -O -Sml1i -W1p >> %ps%

REM Clean up
del .gmt*
if %master%==y cd ..
