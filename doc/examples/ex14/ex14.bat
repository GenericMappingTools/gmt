REM		GMT EXAMPLE 14
REM
REM Purpose:	Showing simple gridding, contouring, and resampling along tracks
REM GMT modules:	blockmean, grdcontour, grdtrack, grdtrend, project, text,
REM GMT modules:	set, plot, surface, subplot
REM DOS calls:	del
REM
gmt begin ex14
	gmt set MAP_GRID_PEN_PRIMARY thinnest,-
	REM calculate mean data and grids
	gmt blockmean @Table_5_11.txt -R0/7/0/7 -I1 > mean.xyz
	gmt surface mean.xyz -Gdata.nc
	gmt grdtrend data.nc -N10 -Ttrend.nc
	gmt project -C0/0 -E7/7 -G0.1 -N > track
	REM Sample along diagonal
	gmt grdtrack track -Gdata.nc -o2,3 > data.d
	gmt grdtrack track -Gtrend.nc -o2,3 > trend.d
	gmt plot -Ra -JX15c/3.5c data.d -Wthick -Bx1 -By50 -BWSne
	gmt plot trend.d -Wthinner,-
	gmt subplot begin 2x2 -M0.1c -Ff15c -BWSne -Yh+1c
		REM First draw network and label the nodes
		gmt plot @Table_5_11.txt -R0/7/0/7 -Sc0.12c -Gblack -c0,0
		gmt text @Table_5_11.txt -D3p/0 -F+f6p+jLM -N
		REM Then draw gmt blockmean cells and label data values using one decimal
		gmt plot mean.xyz -Ss0.12c -Gblack -c0,1
		gmt text -D11p/0 -F+f6p+jLM+z%%.1f -Gwhite -W -C1p -N mean.xyz
		REM Then gmt surface and contour the data
		gmt grdcontour data.nc -C25 -A50 -Gd8c -S4 -c1,0
		gmt plot mean.xyz -Ss0.12c -Gblack
		REM Fit bicubic trend to data and compare to gridded gmt surface
		gmt grdcontour trend.nc -C25 -A50 -Glct/cb -S4 -c1,1
		gmt plot track -Wthick,.
	gmt subplot end
gmt end show
del mean.xyz track trend.nc data.nc data.d trend.d
