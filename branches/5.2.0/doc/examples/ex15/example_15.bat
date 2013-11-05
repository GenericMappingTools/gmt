REM		GMT EXAMPLE 15
REM
REM		$Id$
REM
REM Purpose:	Gridding and clipping when data are missing
REM GMT progs:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo
REM GMT progs:	nearneighbor, pscoast, psmask, pstext, surface
REM DOS calls:	echo, del
REM
echo GMT EXAMPLE 15
set ps=example_15.ps
gmt gmtconvert ship.xyz -bo > ship.b
set region=-R245/255/20/30
gmt nearneighbor %region% -I10m -S40k -Gship.nc ship.b -bi
gmt grdcontour ship.nc -JM3i -P -B2 -BWSne -C250 -A1000 -Gd2i -K > %ps%
REM
gmt blockmedian %region% -I10m ship.b -b > ship_10m.b
gmt surface %region% -I10m ship_10m.b -Gship.nc -bi
gmt psmask %region% -I10m ship.b -J -O -K -T -Glightgray -bi3d -X3.6i >> %ps%
gmt grdcontour ship.nc -J -B -C250 -A1000 -L-8000/0 -Gd2i -O -K >> %ps%
REM
gmt psmask %region% -I10m ship_10m.b -bi3d -J -B -O -K -X-3.6i -Y3.75i >> %ps%
gmt grdcontour ship.nc -J -C250 -A1000 -Gd2i -L-8000/0 -O -K >> %ps%
gmt psmask -C -O -K >> %ps%
REM
gmt grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
gmt grdcontour ship_clipped.nc -J -B -C250 -A1000 -L-8000/0 -Gd2i -O -K -X3.6i >> %ps%
gmt pscoast %region% -J -O -K -Ggray -Wthinnest >> %ps%
gmt grdinfo -C -M ship.nc | gmt psxy -R -J -O -K -Sa0.15i -Wthick -i11,12 >> %ps%
echo -0.3 3.6 Gridding with missing data | gmt pstext -R0/3/0/4 -Jx1i -F+f24p,Helvetica-Bold+jCB -O -N >> %ps%
del ship*.nc
del ship*.b
del .gmt*
