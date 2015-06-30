REM		GMT EXAMPLE 18
REM
REM		$Id$
REM
REM Purpose:	Illustrates volumes of grids inside contours and spatial
REM		selection of data
REM GMT progs:	gmtset, gmtselect, gmtspatial, grdclip, grdcontour, grdgradient, grdimage
REM GMT progs:	grdmath, grdvolume, makecpt, pscoast, psscale, pstext, psxy
REM DOS calls:	gawk, echo, del
REM
echo GMT EXAMPLE 18
set ps=example_18.ps

REM Use spherical projection since SS data define on sphere
gmt gmtset PROJ_ELLIPSOID Sphere FORMAT_FLOAT_OUT %%g

REM Define location of Pratt seamount and the 400 km diameter
echo -142.65 56.25 400 > pratt.d

REM First generate gravity image w/ shading, label Pratt, and draw a circle
REM of radius = 200 km centered on Pratt.

gmt makecpt -Crainbow -T-60/60/120 -Z > grav.cpt
gmt grdgradient AK_gulf_grav.nc -Nt1 -A45 -GAK_gulf_grav_i.nc
gmt grdimage AK_gulf_grav.nc -IAK_gulf_grav_i.nc -JM5.5i -Cgrav.cpt -B2f1 -P -K -X1.5i -Y5.85i > %ps%
gmt pscoast -RAK_gulf_grav.nc -J -O -K -Di -Ggray -Wthinnest >> %ps%
gmt psscale -DjCB+o0/-0.4i+w4i/0.15i+h -R -J -Cgrav.cpt -Bx20f10 -By+l"mGal" -O -K >> %ps%
echo {print $1, $2, "Pratt"} > t
gawk -f t pratt.d | gmt pstext -R -J -O -K -D0.1i/0.1i -F+f12p,Helvetica-Bold+jLB >> %ps%
gmt psxy pratt.d -R -J -O -K -SE- -Wthinnest >> %ps%

REM Then draw 10 mGal contours and overlay 50 mGal contour in green

gmt grdcontour AK_gulf_grav.nc -J -C20 -B2f1 -BWSEn -O -K -Y-4.85i >> %ps%
REM Save 50 mGal contours to individual files, then plot them
gmt grdcontour AK_gulf_grav.nc -C10 -L49/51 -Dsm_%%d_%%c.txt
gmt psxy -R -J -O -K -Wthin,green sm_*.txt >> %ps%
gmt pscoast -R -J -O -K -Di -Ggray -Wthinnest >> %ps%
gmt psxy pratt.d -R -J -O -K -SE- -Wthinnest >> %ps%
REM Only consider closed contours
del sm_*_O.txt

REM Now determine centers of each enclosed seamount exceeding 50 mGal but
REM only plot the ones within 200 km of Pratt seamount.

REM First determine mean location of each closed contour

gmt gmtspatial -Q -fg sm_*_C.txt > centers.d

REM Only plot the ones within 200 km

gmt gmtselect -C200k/pratt.d centers.d -fg | gmt psxy -R -J -O -K -SC0.04i -Gred -Wthinnest >> %ps%
gmt psxy -R -J -O -K -ST0.1i -Gyellow -Wthinnest pratt.d >> %ps%

REM Then report the volume and area of these seamounts only
REM by masking out data outside the 200 km-radius circle
REM and then evaluate area/volume for the 50 mGal contour

gmt grdmath -R -142.65 56.25 SDIST = mask.nc
gmt grdclip mask.nc -Sa200/NaN -Sb200/1 -Gmask.nc
gmt grdmath AK_gulf_grav.nc mask.nc MUL = tmp.nc
echo -148.5	52.75 > tmp
echo -140.5	52.75 >> tmp
echo -140.5	53.75 >> tmp
echo -148.5	53.75 >> tmp
gmt psxy -R -J -A -O -K -L -Wthin -Gwhite tmp >> %ps%
echo {printf "-148 53.08 Areas: %%s km@+2@+\n-148 53.42 Volumes: %%s km@+2@+\n", $2, $3} > t
gmt grdvolume tmp.nc -C50 -Sk | gawk -f t | gmt pstext -R -J -F+f14p,Helvetica-Bold+jLM -O >> %ps%

REM Clean up

del t
del grav.cpt
del sm_*.txt
del *_i.nc
del tmp.nc
del mask.nc
del pratt.d
del center*.*
del .gmt*
del gmt.conf
