REM		GMT EXAMPLE 20
REM
REM Purpose:	Extend GMT to plot custom symbols
REM GMT modules:	coast, plot
REM DOS calls:	echo, del
REM
REM Plot a world-map with volcano symbols of different sizes at hotspot locations
REM using table from Muller et al., 1993, Geology.
gmt begin ex20
	gmt set PROJ_LENGTH_UNIT inch
	gmt coast -Rg -JR22c -B -B+t"Hotspot Islands and Hot Cities" -Gdarkgreen -Slightblue -A5000
	gmt plot @hotspots.txt -Skvolcano -Wthinnest -Gred

	REM Overlay a few bullseyes at NY, Cairo, Perth, and Montevideo
	echo 74W	40.45N	0.5		> cities.txt
	echo 31.15E	30.03N	0.5		>> cities.txt
	echo 115.49E	31.58S	0.5	>> cities.txt
	echo 56.16W	34.9S	0.5		>> cities.txt
	gmt plot cities.txt -Sk@bullseye
	del cities.txt
gmt end show
