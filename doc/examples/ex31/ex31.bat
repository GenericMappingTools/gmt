REM		GMT EXAMPLE 31
REM
REM Purpose:	Illustrate usage of non-default fonts in PostScript
REM GMT modules:	set, coast, plot, text, legend
REM

gmt begin ex31
	REM Set FONTPATH used in image conversion
	REM %~dp0 is the path to the current batch file
	gmt set PS_CONVERT="C-sFONTPATH=%~dp0fonts"
	REM create file PSL_custom_fonts.txt in current working directory
	REM and add PostScript font names of Linux Biolinum and Libertine
	echo LinBiolinumO   0.700 0 > PSL_custom_fonts.txt
	echo LinBiolinumOI  0.700 0 >> PSL_custom_fonts.txt
	echo LinBiolinumOB  0.700 0 >> PSL_custom_fonts.txt
	echo LinLibertineOB 0.700 0 >> PSL_custom_fonts.txt

	gmt which -G @europe-capitals-ru.csv > capitals.txt
	set /p capitals=<capitals.txt
	REM common settings
	gmt set FORMAT_GEO_MAP ddd:mm:ssF MAP_DEGREE_SYMBOL colon MAP_TITLE_OFFSET 20p ^
		MAP_GRID_CROSS_SIZE_PRIMARY 0.4c PS_LINE_JOIN round PS_CHAR_ENCODING ISO-8859-5 ^
		FONT LinBiolinumO FONT_TITLE 24p,LinLibertineOB MAP_ANNOT_OBLIQUE 42

	REM map of countries
	gmt coast -R-7/31/64/66+r -JL15/50/40/60/16c -Bx10g10 -By5g5 -B+t"Europe\072 Countries and Capital Cities" -A250 -Slightblue -Glightgreen -W0.25p -N1/1p,white
	REM mark capitals
	gmt plot @europe-capitals-ru.csv -i0,1 -Sc0.15c -G196/80/80
	REM small EU cities
	gawk "BEGIN {FS=\",\"} $4 !=\"\" && $4 <= 1000000 {print $1, $2}" %capitals% | gmt plot -Sc0.15c -W0.25p
	REM big EU cities
	gawk "BEGIN {FS=\",\"} $4 > 1000000 {print $1, $2}" %capitals% | gmt plot -Sc0.15c -W1.25p
	REM label big EU cities
	gawk "BEGIN {FS=\",\"} $4 > 1000000 {print $1, $2, $3}" %capitals% | gmt text -F+f7p,LinBiolinumOI+jBL -Dj0.1c -Gwhite -C5%%+tO -Qu

	REM construct legend
	echo G -0.1c > legend.txt
	echo H 10p,LinBiolinumOB Population of the European Union capital cities >> legend.txt
	echo G 0.15c >> legend.txt
	echo N 2 >> legend.txt
	echo S 0.15c c 0.15c 196/80/80 0.25p 0.5c ^< 1 Million inhabitants >> legend.txt
	echo S 0.15c c 0.15c 196/80/80 1.25p 0.5c ^> 1 Million inhabitants >> legend.txt
	echo N 1 >> legend.txt
	echo G 0.15c >> legend.txt
	echo L 8p,LinBiolinumOB L Population in Millions >> legend.txt
	echo N 6 >> legend.txt

	REM append city names and population to legend
	gawk "BEGIN {FS=\",\"; f=\"L 8p,LinBiolinumO L\"} $4 > 1000000 {printf \"%%s %%s:\n%%s %%.2f\n\", f, $3, f, $4/1e6}" %capitals% >> legend.txt

	REM reduce annotation font size for legend
	gmt set FONT_ANNOT_PRIMARY 8p

	REM plot legend
	gmt legend -DjTR+o0.1c+w8.0c+l1.2 -C0.3c/0.4c -F+p+gwhite legend.txt

	del PSL_custom_fonts.txt legend.txt ex31CropNoLogo.eps europe-capitals-ru.csv
gmt end show
