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
	gmt subplot begin 2x2 -M0.1i/0.05i -Fs2.9i/0 %region% -JM2.9i -BWSne -T"Gridding with missing data"
		REM   Raw nearest neighbor contouring
		gmt nearneighbor %region% -I10m -S40k -Gship.nc ship.b -bi
		gmt grdcontour ship.nc -JM -C250 -A1000 -Gd2i -c1,0
		REM   Grid via surface but mask out area with no data using coastlines
		gmt blockmedian ship.b -b3d > ship_10m.b
		gmt surface ship_10m.b -Gship.nc -bi
		gmt mask -I10m ship.b -T -Glightgray -bi3d -c1,1
		gmt grdcontour ship.nc -C250 -L-8000/0 -A1000 -Gd2i
		REM   Grid via surface but mask out area with no data
		gmt mask -I10m ship_10m.b -bi3d -c0,0
		gmt grdcontour ship.nc -C250 -A1000 -L-8000/0 -Gd2i
		gmt mask -C
		REM   Clip data above sealevel then overlay land
		gmt grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
		gmt grdcontour ship_clipped.nc -C250 -A1000 -L-8000/0 -Gd2i -c0,1
		gmt coast -Ggray -Wthinnest
		gmt grdinfo -Cn -M ship.nc | gmt plot -Sa0.15i -Wthick -i10,11
	gmt subplot end
gmt end show
del ship.b ship_10m.b ship.nc ship_clipped.nc region.txt
