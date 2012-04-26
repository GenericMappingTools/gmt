REM		GMT EXAMPLE 15
REM
REM		$Id$
REM
REM Purpose:	Gridding and clipping when data are missing
REM GMT progs:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo, minmax
REM GMT progs:	nearneighbor, pscoast, psmask, pstext, surface
REM DOS calls:	echo, del
REM
echo GMT EXAMPLE 15
set ps=example_15.ps
gmtconvert ship.xyz -bod > ship.b
set region=-R245/255/20/30
nearneighbor %region% -I10m -S40k -Gship.nc ship.b -bi3
grdcontour ship.nc -JM3i -P -B2WSne -C250 -A1000 -Gd2i -K -U"Example 15 in Cookbook" > %ps%
REM
blockmedian %region% -I10m ship.b -bi3 -bod > ship_10m.b
surface %region% -I10m ship_10m.b -Gship.nc -bi3
psmask %region% -I10m ship.b -J -O -K -T -Glightgray -bi3 -X3.6i >> %ps%
grdcontour ship.nc -J -B -C250 -A1000 -L-8000/0 -Gd2i -O -K >> %ps%
REM
psmask %region% -I10m ship_10m.b -bi3 -J -B -O -K -X-3.6i -Y3.75i >> %ps%
grdcontour ship.nc -J -C250 -A1000 -Gd2i -L-8000/0 -O -K >> %ps%
psmask -C -O -K >> %ps%
REM
grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
grdcontour ship_clipped.nc -J -B -C250 -A1000 -L-8000/0 -Gd2i -O -K -X3.6i >> %ps%
pscoast %region% -J -O -K -Ggray -Wthinnest >> %ps%
grdinfo -C -M ship.nc | psxy -R -J -O -K -Sa0.15i -Wthick -i11,12 >> %ps%
echo -0.3 3.6 Gridding with missing data | pstext -R0/3/0/4 -Jx1i -F+f24p,Helvetica-Bold+jCB -O -N >> %ps%
del ship*.nc
del ship*.b
del .gmt*
