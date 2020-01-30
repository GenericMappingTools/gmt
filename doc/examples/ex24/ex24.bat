REM		GMT EXAMPLE 24
REM
REM Purpose:	Extract subsets of data based on geospatial criteria
REM GMT modules:	select, coast, plot, info
REM DOS calls:	echo, del
REM
REM Highlight oceanic earthquakes within 3000 km of Hobart and > 1000 km from dateline
gmt begin ex24
	echo 147:13 -42:48 6000 > point.txt
	echo ^> Our proxy for the dateline > dateline.txt
	echo 180	0 >> dateline.txt
	echo 180	-90 >> dateline.txt
	gmt info -I10 @oz_quakes_24.txt > R.txt
	set /p R=<R.txt
	gmt coast %R% -JM22c -Gtan -Sdarkblue -Wthin,white -Dl -A500 -Ba20f10g10 -BWeSn
	gmt plot @oz_quakes_24.txt -Sc0.1c -Gred
	gmt select @oz_quakes_24.txt -Ldateline.txt+d1000k -Nk/s -Cpoint.txt+d3000k -fg -Il | gmt plot -Sc0.1c -Ggreen
	gmt plot point.txt -SE- -Wfat,white
	gmt text point.txt -F+f14p,Helvetica-Bold,white+jLT+tHobart -Dj7p
	gmt plot point.txt -Wfat,white -S+0.5c
	gmt plot dateline.txt -Wfat,white -A
	del point.txt dateline.txt
gmt end show
