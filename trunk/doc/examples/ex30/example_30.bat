REM             GMT EXAMPLE 30
REM             $Id$
REM
REM Purpose:	Show graph mode and math angles
REM GMT progs:	gmtmath, psbasemap, pstext and psxy
REM DOS calls:	del, echo
REM

echo GMT EXAMPLE 30
set ps=example_30.ps
gmtset PS_CHAR_ENCODING	Standard+

psbasemap -R0/360/-1.25/1.75 -JX8i/6i -B90f30:,-\312:/1g10:."Two Trigonometric Functions":WS -K -U"Example 30 in Cookbook" --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5 > %ps%

REM Draw sine an cosine curves

gmtmath -T0/360/0.1 T COSD = | psxy -R -J -O -K -W3p >> %ps%
gmtmath -T0/360/0.1 T SIND = | psxy -R -J -O -K -W3p,0_6:0 --PS_LINE_CAP=round >> %ps%

REM Indicate the x-angle = 120 degrees
echo 120 -1.25 > tmp
echo 120 1.25 >> tmp
psxy -R -J -O -K -W0.5p,- tmp >> %ps%

echo 360 1 18p,Times-Roman RB x = cos(@%%12%%a@%%%%) > tmp
echo 360 0 18p,Times-Roman RB y = sin(@%%12%%a@%%%%) >> tmp
echo 120 -1.25 14p,Times-Roman LB 120\312 >> tmp
echo 370 -1.35 24p,Symbol LT a >> tmp
echo -5 1.85 24p,Times-Roman RT x,y >> tmp
pstext -R -J -O -K -Dj0.2c -N tmp -F+f+j >> %ps%

REM Draw a circle and indicate the 0-70 degree angle

echo 0 0 | psxy -R-1/1/-1/1 -Jx1.5i -O -K -X3.625i -Y2.75i -Sc2i -W1p -N >> %ps%
echo -1	0 > tmp
echo 1	0 >> tmp
psxy -R -J -O -K -W tmp >> %ps%
echo 0	-1 > tmp
echo 0	1 >> tmp
psxy -R -J -O -K -W tmp >> %ps%
echo 0	0 > tmp
echo 1	0 >> tmp
psxy -R -J -O -K -W1p tmp >> %ps%
echo 0	0 > tmp
echo -0.5	0.866025 >> tmp
psxy -R -J -O -K -W1p tmp >> %ps%
echo -0.3333	0 > tmp
echo 0	0 >> tmp
psxy -R -J -O -K -W2p tmp >> %ps%
echo -0.3333	0.57735 > tmp
echo -0.3333	0 >> tmp
psxy -R -J -O -K -W2p tmp >> %ps%

echo -0.16666 0 12p,5 0 CT x > tmp
echo -0.3333 0.2888675 12p,Times-Roman 0 RM y >> tmp
echo 0.22 0.27 12p,Symbol -30 CB a >> tmp
echo -0.33333 0.6 12p,Times-Roman 30 LB 120\312 >> tmp
pstext -R -J -O -K -Dj0.05i tmp -F+f+a+j >> %ps%

echo 0 0 0 120 | psxy -R -J -O -Sml1i -W1p -Gblack >> %ps%

REM Clean up
del .gmt*
