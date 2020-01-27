REM               GMT EXAMPLE 46
REM
REM Purpose:      Illustrate use of solar to plot day/night terminators
REM GMT modules:  solar, coast, plot
REM

gmt begin ex46
	gmt coast -Rd -JKs0/25c -A5000 -W0.5p -N1/0.5p,gray -S175/210/255 -Bafg --MAP_FRAME_TYPE=plain
	gmt solar -Td+d2016-02-09T16:00:00 -Gnavy@95
	gmt solar -Tc+d2016-02-09T16:00:00 -Gnavy@85
	gmt solar -Tn+d2016-02-09T16:00:00 -Gnavy@80
	gmt solar -Ta+d2016-02-09T16:00:00 -Gnavy@80
	gmt solar -I+d2016-02-09T16:00:00 -C | gmt plot -Sk@sunglasses/1.5c
gmt end show
