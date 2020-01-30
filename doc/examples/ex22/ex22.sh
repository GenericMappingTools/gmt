#!/usr/bin/env bash
#		GMT EXAMPLE 22
#
# Purpose:	Automatic map of last month of world-wide seismicity
# GMT modules:	set, coast, plot, legend
# Unix progs:	cat, sed, awk, wget|curl
#
gmt begin ex22
	gmt set FONT_ANNOT_PRIMARY 10p FONT_TITLE 18p FORMAT_GEO_MAP ddd:mm:ssF

	# Get the data (-s silently) from USGS using the curl
	# Hardwired here to the month of October, 2017
	# SITE="https://earthquake.usgs.gov/fdsnws/event/1/query.csv"
	# TIME="starttime=2017-09-01%2000:00:00&endtime=2017-10-01%2000:00:00"
	# MAG="minmagnitude=3"
	# ORDER="orderby=magnitude"
	# URL="${SITE}?${TIME}&${MAG}&${ORDER}"
	# curl -s $URL > usgs_quakes_22.txt

	# Count the number of events (to be used in title later. one less due to header)
	file=`gmt which @usgs_quakes_22.txt -G`
	n=`gmt info $file -h1 -Fi -o2`

	# Pull out the first and last timestamp to use in legend title
	first=`gmt info -h1 -f0T -i0 $file -C --TIME_UNIT=d -I1 -o0 --FORMAT_CLOCK_OUT=-`
	last=`gmt info -h1 -f0T -i0 $file -C --TIME_UNIT=d -I1 -o1 --FORMAT_CLOCK_OUT=-`

	# Assign a string that contains the current user @ the current computer node.
	# Note that two @@ is needed to print a single @ in gmt text:

	#set me = "$user@@`hostname`"
	me="GMT guru @@ GMTbox"

	# Create standard seismicity color table
	gmt makecpt -Cred,green,blue -T0,100,300,10000 -N

	# Start plotting. First lay down map, then plot quakes with size = magnitude * 0.015":
	gmt coast -Rg -JK180/22c -B45g30 -B+t"World-wide earthquake activity" -Gburlywood -Slightblue -A1000
	gmt plot -C -Sci -Wfaint -hi1 -i2,1,3,4+s0.015 $file

	# Create legend input file for NEIS quake plot
	cat > neis.legend <<- END
	H 16p,Helvetica-Bold $n events during $first to $last
	D 0 1p
	N 3
	V 0 1p
	S 0.25c c 0.25c red   0.25p 0.5c Shallow depth (0-100 km)
	S 0.25c c 0.25c green 0.25p 0.5c Intermediate depth (100-300 km)
	S 0.25c c 0.25c blue  0.25p 0.5c Very deep (> 300 km)
	D 0 1p
	V 0 1p
	N 7
	V 0 1p
	S 0.25c c 0.15c - 0.25p 0.75c M 3
	S 0.25c c 0.20c - 0.25p 0.75c M 4
	S 0.25c c 0.25c - 0.25p 0.75c M 5
	S 0.25c c 0.30c - 0.25p 0.75c M 6
	S 0.25c c 0.35c - 0.25p 0.75c M 7
	S 0.25c c 0.40c - 0.25p 0.75c M 8
	S 0.25c c 0.45c - 0.25p 0.75c M 9
	D 0 1p
	V 0 1p
	N 1
	END

	# Put together a reasonable legend text, and add logo and user's name:
	cat <<- END >> neis.legend
	G 0.25l
	P
	T USGS/NEIS most recent earthquakes for the last month.  The data were
	T obtained automatically from the USGS Earthquake Hazards Program page at
	T @_https://earthquake.usgs.gov@_.  Interested users may also receive email alerts
	T from the USGS.
	T This script could be called monthly to update the latest information.
	G 0.9c
	# Add USGS logo
	I @USGS.png 1i RT
	G -0.75c
	L 12p,Times-Italic LB $me
	END

	# OK, now we can actually run gmt legend.  We center the legend below the map.
	# Trial and error shows that 1.7i is a good legend height:
	gmt legend -DJBC+o0/1c+w18c/4.2c -F+p+glightyellow neis.legend

	rm neis.legend usgs_quakes_22.txt
gmt end show
