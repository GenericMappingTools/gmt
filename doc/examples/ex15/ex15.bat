REM		GMT EXAMPLE 15
REM
REM Purpose:	Gridding and clipping when data are missing
REM GMT modules:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo
REM GMT modules:	info, nearneighbor, coast, mask, surface, plot
REM DOS calls: del
REM
gmt begin ex15
	gmt convert @ship_15.txt -bo > ship.b
	gmt info ship.b -I1 -bi3d > region.txt
	set /p region=<region.txt
	gmt subplot begin 2x2 -M0.3c/0.1c -Fs7.5c/0 %region% -JM7.5c -BWSne -T"Gridding with missing data"
		REM   Raw nearest neighbor contouring
		gmt nearneighbor %region% -I10m -S40k -Gship.nc ship.b -bi
		gmt grdimage ship.nc -c1,0
		gmt plot ship.b -bi3d -Sc0.005c
		REM   Grid via surface but mask out area with no data using coastlines
		gmt mask -I10m ship.b -T -Gorange -bi3d -c1,1
		gmt plot ship.b -bi3d -Sc0.005c
		REM   Grid via surface but mask out area with no data
		gmt blockmedian ship.b -b3d > ship_10m.b
		gmt surface ship_10m.b -Gship.nc -bi
		gmt mask -I10m ship_10m.b -bi3d -c0,0
		gmt grdimage ship.nc
		gmt mask -C
		REM   Clip data above sealevel then overlay land
		gmt grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
		gmt grdimage ship_clipped.nc -c0,1
		gmt coast -Ggray -Wthinnest
		gmt grdinfo -Cn -M ship.nc | gmt plot -Sa0.5c -Wthick -i10,11 -Gred
	gmt subplot end
gmt end show
del ship.b ship_10m.b ship.nc ship_clipped.nc region.txt
