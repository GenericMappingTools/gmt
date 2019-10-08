REM		GMT EXAMPLE 22
REM
REM Purpose:	Automatic map of last month of world-wide seismicity
REM GMT modules:	set, coast, plot, legend
REM
gmt begin ex22
	gmt set FONT_ANNOT_PRIMARY 10p FONT_TITLE 18p FORMAT_GEO_MAP ddd:mm:ssF

	REM Get the data (-s silently) from USGS using the curl
	REM Hardwired here to the month of October, 2017
	REM SITE="https://earthquake.usgs.gov/fdsnws/event/1/query.csv"
	REM TIME="starttime=2017-09-01%2000:00:00&endtime=2017-10-01%2000:00:00"
	REM MAG="minmagnitude=3"
	REM ORDER="orderby=magnitude"
	REM URL="${SITE}?${TIME}&${MAG}&${ORDER}"
	REM curl -s $URL > usgs_quakes_22.txt

	REM Count the number of events (to be used in title later. one less due to header)
	gmt which @usgs_quakes_22.txt -G > file.txt
	set /p file=<file.txt

	gmt info %file% -h1 -Fi -o2 > n.txt
	set /p n=<n.txt

	REM Pull out the first and last timestamp to use in legend title
	gmt info -h1 -f0T -i0 %file% -C --TIME_UNIT=d -I1 -o0 --FORMAT_CLOCK_OUT=- > first.txt
	set /p first=<first.txt
	gmt info -h1 -f0T -i0 %file% -C --TIME_UNIT=d -I1 -o1 --FORMAT_CLOCK_OUT=- > last.txt
	set /p last=<last.txt

	REM Assign a string that contains the current user @ the current computer node.
	REM Note that two @@ is needed to print a single @ in gmt text:

	set me=GMT guru @@ GMTbox

	REM Create standard seismicity color table
	gmt makecpt -Cred,green,blue -T0,100,300,10000 -N

	REM Start plotting. First lay down map, then plot quakes with size = magnitude * 0.015":
	gmt coast -Rg -JK180/9i -B45g30 -B+t"World-wide earthquake activity" -Gburlywood -Slightblue -A1000 -Y2.75i
	gmt plot -C -Sci -Wfaint -hi1 -i2,1,3,4+s0.015 %file%

	REM Create legend input file for NEIS quake plot
	echo H 16p,Helvetica-Bold %n% events during %first% to %last% > neis.legend
	echo D 0 1p >> neis.legend
	echo N 3 >> neis.legend
	echo V 0 1p >> neis.legend
	echo S 0.1i c 0.1i red 0.25p 0.2i Shallow depth (0-100 km) >> neis.legend
	echo S 0.1i c 0.1i green 0.25p 0.2i Intermediate depth (100-300 km) >> neis.legend
	echo S 0.1i c 0.1i blue 0.25p 0.2i Very deep (> 300 km) >> neis.legend
	echo D 0 1p >> neis.legend
	echo V 0 1p >> neis.legend
	echo N 7 >> neis.legend
	echo V 0 1p >> neis.legend
	echo S 0.1i c 0.06i - 0.25p 0.3i M 3 >> neis.legend
	echo S 0.1i c 0.08i - 0.25p 0.3i M 4 >> neis.legend
	echo S 0.1i c 0.10i - 0.25p 0.3i M 5 >> neis.legend
	echo S 0.1i c 0.12i - 0.25p 0.3i M 6 >> neis.legend
	echo S 0.1i c 0.14i - 0.25p 0.3i M 7 >> neis.legend
	echo S 0.1i c 0.16i - 0.25p 0.3i M 8 >> neis.legend
	echo S 0.1i c 0.18i - 0.25p 0.3i M 9 >> neis.legend
	echo D 0 1p >> neis.legend
	echo V 0 1p >> neis.legend
	echo N 1 >> neis.legend

	REM Put together a reasonable legend text, and add logo and user's name:
	echo G 0.25l >> neis.legend
	echo P >> neis.legend
	echo T USGS/NEIS most recent earthquakes for the last month.  The data were >> neis.legend
	echo T obtained automatically from the USGS Earthquake Hazards Program page at >> neis.legend
	echo T @_https://earthquake.usgs.gov@_.  Interested users may also receive email alerts >> neis.legend
	echo T from the USGS. >> neis.legend
	echo T This script could be called monthly to update the latest information. >> neis.legend
	echo G 0.4i >> neis.legend
	echo # Add USGS logo >> neis.legend
	echo I @USGS.png 1i RT >> neis.legend
	echo G -0.3i >> neis.legend
	echo L 12p,Times-Italic LB %me% >> neis.legend

	REM OK, now we can actually run gmt legend.  We center the legend below the map.
	REM Trial and error shows that 1.7i is a good legend height:
	gmt legend -DJBC+o0/0.4i+w7i/1.7i -F+p+glightyellow neis.legend

	del neis.legend usgs_quakes_22.txt
gmt end show
