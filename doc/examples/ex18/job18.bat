REM		GMT EXAMPLE 18
REM
REM		$Id: job18.bat,v 1.17 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrates volumes of grids inside contours and spatial
REM		selection of data
REM GMT progs:	gmtset, gmtselect, grdclip, grdcontour, grdgradient, grdimage
REM GMT progs:	grdmath, grdvolume, makecpt, pscoast, psscale, pstext, psxy
REM DOS calls:	gawk, echo, del
REM
echo GMT EXAMPLE 18
set master=y
if exist job18.bat set master=n
if %master%==y cd ex18

REM Use spherical projection since SS data define on sphere
gmtset ELLIPSOID Sphere D_FORMAT %%lg

REM Define location of Pratt seamount
echo -142.65 56.25 > pratt.d

REM First generate gravity image w/ shading, label Pratt, and draw a circle
REM of radius = 200 km centered on Pratt.

makecpt -Crainbow -T-60/60/10 -Z > grav.cpt
grdgradient AK_gulf_grav.nc -Nt1 -A45 -GAK_gulf_grav_i.nc
grdimage AK_gulf_grav.nc -IAK_gulf_grav_i.nc -JM5.5i -Cgrav.cpt -B2f1 -P -K -X1.5i -Y5.85i > ..\example_18.ps
pscoast -RAK_gulf_grav.nc -J -O -K -Di -Ggray -Wthinnest >> ..\example_18.ps
psscale -D2.75i/-0.4i/4i/0.15ih -Cgrav.cpt -B20f10/:mGal: -O -K >> ..\example_18.ps
echo {print $1, $2, 12, 0, 1, "LB", "Pratt"} > t
gawk -f t pratt.d | pstext -R -J -O -K -D0.1i/0.1i >> ..\example_18.ps
gawk "{print $1, $2, 0, 400, 400}" pratt.d | psxy -R -J -O -K -SE -Wthinnest >> ..\example_18.ps

REM Then draw 10 mGal contours and overlay 50 mGal contour in green

grdcontour AK_gulf_grav.nc -J -C20 -B2f1WSEn -O -K -Y-4.85i -U/-1.25i/-0.75i/"Example 18 in Cookbook" >> ..\example_18.ps
grdcontour AK_gulf_grav.nc -J -C10 -L49/51 -O -K -D- -Wcthin,green >> ..\example_18.ps
pscoast -R -J -O -K -Di -Ggray -Wthinnest >> ..\example_18.ps
gawk "{print $1, $2, 0, 400, 400}" pratt.d | psxy -R -J -O -K -SE -Wthinnest >> ..\example_18.ps
REM Only consider closed contours
del C*_e.xyz

REM Now determine centers of each enclosed seamount exceeding 50 mGal but
REM only plot the ones within 200 km of Pratt seamount.

REM First determine mean location of each closed contour and
REM add it to the file centers.d using center.awk script

for %%f in (C*.xyz) do gawk "BEGIN{x=0;y=0;n=0};{x+=$1;y+=$2;n++};END{print x/n,y/n}" %%f >> centers.d

REM Only plot the ones within 200 km

gmtselect -C200/pratt.d centers.d -fg | psxy -R -J -O -K -SC0.04i -Gred -Wthinnest >> ..\example_18.ps
psxy -R -J -O -K -ST0.1i -Gyellow -Wthinnest pratt.d >> ..\example_18.ps

REM Then report the volume and area of these seamounts only
REM by masking out data outside the 200 km-radius circle
REM and then evaluate area/volume for the 50 mGal contour

grdmath -R -142.65 56.25 GDIST = mask.nc
grdclip mask.nc -Sa200/NaN -Sb200/1 -Gmask.nc
grdmath AK_gulf_grav.nc mask.nc MUL = tmp.nc
echo -148.5	52.75 > tmp
echo -140.5	52.75 >> tmp
echo -140.5	53.75 >> tmp
echo -148.5	53.75 >> tmp
psxy -R -J -A -O -K -L -Wthin -Gwhite tmp >> ..\example_18.ps
echo {printf "-148 53.08 14 0 1 LM Areas: %%s km@+2@+\n-148 53.42 14 0 1 LM Volumes: %%s km@+2@+\n", $2, $3} > t
grdvolume tmp.nc -C50 -Sk | gawk -f t | pstext -R -J -O >> ..\example_18.ps

REM Clean up

del t
del grav.cpt
del C*.xyz
del *_i.nc
del tmp.nc
del mask.nc
del pratt.d
del center*.*
del .gmt*
if %master%==y cd ..
