REM		GMT EXAMPLE 21
REM
REM Purpose:	Plot a time-series
REM GMT modules:	set, convert, info, basemap, plot
REM
gmt begin ex21
	REM File has time stored as dd-Mon-yy so set input format to match it
	gmt set FORMAT_DATE_IN dd-o-yy FORMAT_DATE_MAP o FONT_ANNOT_PRIMARY +10p
	gmt set FORMAT_TIME_PRIMARY_MAP abbreviated PS_CHAR_ENCODING ISOLatin1+

	REM Create a suitable -R string
	set R=-R1999-08-04T12:00:00/2008-01-29T12:00:00/0/300

	REM Lay down the basemap:
	gmt basemap %R% -JX22c/15c -Bsx1Y -Bpxa3Of1o -Bpy50+p"$ " -BWSen+t"RedHat (RHT) Stock Price Trend since IPO"+glightgreen

	REM Plot main window with open price as red line over yellow envelope of low/highs
	gmt set FORMAT_DATE_OUT dd-o-yy
	gmt convert -o0,2 -f0T @RHAT_price.csv > RHAT.env
	gmt convert -o0,3 -f0T -I -T @RHAT_price.csv >> RHAT.env
	gmt plot -Gyellow RHAT.env
	gmt plot @RHAT_price.csv -Wthin,red

	REM Draw P Wessel's purchase price as line and label it.  Note we temporary switch
	REM back to default yyyy-mm-dd format since that is what gmt info gave us.
	echo 05-May-00	0 > RHAT.pw
	echo 05-May-00	300 >> RHAT.pw
	gmt plot RHAT.pw -Wthinner,-
	echo 01-Jan-99	25 > RHAT.pw
	echo 01-Jan-02	25 >> RHAT.pw
	gmt plot RHAT.pw -Wthick,-
	gmt set FORMAT_DATE_IN yyyy-mm-dd
	echo 1999-08-11T00:00:00 25 PW buy | gmt text -D4c/0.1c -N -F+f12p,Bookman-Demi+jLB
	gmt set FORMAT_DATE_IN dd-o-yy

	REM Draw P Wessel's sales price as line and label it.
	echo 25-Jun-07	0 > RHAT.pw
	echo 25-Jun-07	300 >> RHAT.pw
	gmt plot RHAT.pw -Wthinner,-
	echo 01-Aug-06	23.8852 > RHAT.pw
	echo 01-Jan-08	23.8852 >> RHAT.pw
	gmt plot RHAT.pw -Wthick,-
	gmt set FORMAT_DATE_IN yyyy-mm-dd
	echo 2008-01-29T12:00:00 23.8852 PW sell | gmt text -Dj1.6c/0.1c -N -F+f12p,Bookman-Demi+jRB
	gmt set FORMAT_DATE_IN dd-o-yy

	REM Get smaller region for inset for trend since 2004
	set R=-R2004T/2007-12-31T00:00:00/0/40

	REM Lay down the basemap, using Finnish annotations and place the inset in the upper right
	gmt basemap --GMT_LANGUAGE=fi %R% -JX15c/7c -Bpxa3Of3o -Bpy10+p"$ " -BESw+glightblue -Bsx1Y -X7c -Y8c

	REM Again, plot close price as red line over yellow envelope of low/highs
	gmt plot -Gyellow RHAT.env
	gmt plot @RHAT_price.csv -Wthin,red

	REM Draw P Wessel's sales price as dashed line
	gmt plot RHAT.pw -Wthick,-

	REM Mark sales date
	echo 25-Jun-07	0 > RHAT.pw
	echo 25-Jun-07	300 >> RHAT.pw
	gmt plot RHAT.pw -Wthinner,-

	REM Clean up after ourselves:
	del RHAT.*
gmt end show
