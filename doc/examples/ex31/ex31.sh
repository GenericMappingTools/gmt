#!/usr/bin/env bash
#		GMT EXAMPLE 31
#
# Purpose:	Illustrate usage of non-default fonts in PostScript
# GMT modules:	set, coast, plot, text, legend
# Unix progs:	awk, cat, rm
#

# set AWK to awk if undefined
AWK=${AWK:-awk}

gmt begin ex31
	# create file PSL_custom_fonts.txt in current working directory
	# and add PostScript font names of Linux Biolinum and Libertine
	$AWK '{print $1, 0.700, 0}' <<- EOF > PSL_custom_fonts.txt
	LinBiolinumO
	LinBiolinumOI
	LinBiolinumOB
	LinLibertineOB
	EOF

	capitals=$(gmt which -G @europe-capitals-ru.csv)
	# common settings
	gmt set FORMAT_GEO_MAP ddd:mm:ssF MAP_DEGREE_SYMBOL colon MAP_TITLE_OFFSET 20p \
		MAP_GRID_CROSS_SIZE_PRIMARY 0.4c PS_LINE_JOIN round PS_CHAR_ENCODING ISO-8859-5 \
		FONT LinBiolinumO FONT_TITLE 24p,LinLibertineOB MAP_ANNOT_OBLIQUE 42

	# map of countries
	gmt coast -R-7/31/64/66+r -JL15/50/40/60/16c -Bx10g10 -By5g5 -B+t"Europe\072 Countries and Capital Cities" -A250 \
		-Slightblue -Glightgreen -W0.25p -N1/1p,white
	# mark capitals
	gmt plot @europe-capitals-ru.csv -i0,1 -Sc0.15c -G196/80/80
	# small EU cities
	$AWK 'BEGIN {FS=","} $4 !="" && $4 <= 1000000 {print $1, $2}' $capitals | gmt plot -Sc0.15c -W0.25p
	# big EU cities
	$AWK 'BEGIN {FS=","} $4 > 1000000 {print $1, $2}' $capitals | gmt plot -Sc0.15c -W1.25p
	# label big EU cities
	$AWK 'BEGIN {FS=","} $4 > 1000000 {print $1, $2, $3}' $capitals | gmt text -F+f7p,LinBiolinumOI+jBL -Dj0.1c -Gwhite -C5%+tO -Qu

	# construct legend
	cat <<- EOF > legend.txt
	G -0.1c
	H 10p,LinBiolinumOB Population of the European Union capital cities
	G 0.15c
	N 2
	S 0.15c c 0.15c 196/80/80 0.25p 0.5c < 1 Million inhabitants
	S 0.15c c 0.15c 196/80/80 1.25p 0.5c > 1 Million inhabitants
	N 1
	G 0.15c
	L 8p,LinBiolinumOB L Population in Millions
	N 6
	EOF

	# append city names and population to legend
	$AWK 'BEGIN {FS=","; f="L 8p,LinBiolinumO L"}
	  $4 > 1000000 {printf "%s %s:\n%s %.2f\n", f, $3, f, $4/1e6}' $capitals >> legend.txt

	# reduce annotation font size for legend
	gmt set FONT_ANNOT_PRIMARY 8p

	# plot legend
	gmt legend -DjTR+o0.1c+w8.0c+l1.2 -C0.3c/0.4c -F+p+gwhite legend.txt

	rm -f PSL_custom_fonts.txt legend.txt ex31CropNoLogo.eps europe-capitals-ru.csv
gmt end show
