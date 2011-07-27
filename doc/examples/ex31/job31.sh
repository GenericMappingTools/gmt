#!/bin/bash
#		GMT EXAMPLE 31
#		$Id$
#
# Purpose:	Illustrate usage of non-default fonts in PostScript
# GMT progs:	gmtset, pscoast, psxy, pstext, pslegend
# Unix progs:	gs, awk, cat, rm
#
. ../functions.sh
file=../example_31
ps=${file}.ps
ps_outlined=${file}_outlined.ps
eps_outlined=${file}_outlined.eps

# create file CUSTOM_font_info.d in current working directory
# and add PostScript font names of Linux Biolinum and Libertine
$AWK '{print $1, 0.700, 0}' << EOF > CUSTOM_font_info.d
LinBiolinumO
LinBiolinumOI
LinBiolinumOB
LinLibertineOB
EOF

# common settings
gmtset FORMAT_GEO_MAP ddd:mm:ssF \
MAP_DEGREE_SYMBOL degree \
MAP_TITLE_OFFSET 20p \
MAP_GRID_CROSS_SIZE_PRIMARY 0.4c \
PS_LINE_JOIN round \
PS_CHAR_ENCODING ISO-8859-5 \
FONT LinBiolinumO \
FONT_TITLE 24p,LinLibertineOB \
MAP_ANNOT_OBLIQUE 42

# map of countries
pscoast -Di -R-7/31/64/66/r -JL15/50/40/60/16c -P \
-B10g10/5g5:."Europe\072 Countries and Capital Cities": -A250 \
-U"Example 31 in Cookbook" -Slightblue -Glightgreen -W0.25p -N1/1p,white -K > $ps

# mark capitals
psxy europe-capitals-ru.csv -R -J -i0,1 \
-Sc0.15c -G196/80/80 -O -K >> $ps

# small EU cities
$AWK 'BEGIN {FS=","} $4 !="" && $4 <= 1000000 {print $1, $2}' europe-capitals-ru.csv | \
psxy -R -J -Sc0.15c -W0.25p -O -K >> $ps

# big EU cities
$AWK 'BEGIN {FS=","} $4 > 1000000 {print $1, $2}' europe-capitals-ru.csv | \
psxy -R -J -Sc0.15c -W1.25p -O -K >> $ps

# label big EU cities
$AWK 'BEGIN {FS=","} $4 > 1000000 {print $1, $2, $3}' europe-capitals-ru.csv | \
pstext -R -J -F+f7p,LinBiolinumOI+jBL -Dj0.1c -Gwhite -C5% -Qu -TO -O -K >> $ps

# construct legend
cat << EOF > legend.txt
G -0.1c
H 10 LinBiolinumOB Population of the European Union capital cities
G 0.15c
N 2
S 0.15c c 0.15c 196/80/80 0.25p 0.5c < 1 Million inhabitants
S 0.15c c 0.15c 196/80/80 1.25p 0.5c > 1 Million inhabitants
N 1
G 0.15c
L 8 LinBiolinumOB L Population in Millions 
N 6
EOF

# append city names and population to legend
$AWK 'BEGIN {FS=","; f="L 8 LinBiolinumO L"}
  $4 > 1000000 {printf "%s %s:\n%s %.2f\n", f, $3, f, $4/1e6}' \
  europe-capitals-ru.csv >> legend.txt

# reduce annotation font size for legend
gmtset FONT_ANNOT_PRIMARY 8p

# plot legend
pslegend -R -J -Gwhite -Dx7.9c/12.6c/8.0c/3.4c/BL \
-C0.3c/0.4c -L1.2 -F -O legend.txt >> $ps

# make a PostScript and a PDF file with outlined fonts
# unfortunately ps2raster won't be able to crop that file correctly anymore
# use Heiko Oberdiek's pdfcrop (http://code.google.com/p/pdfcrop2/) instead
# or crop with ps2raster -A -Te before
#
# a. remove GMT logo and crop EPS:
#ps2raster -P -Au -Te -C-sFONTPATH="${PWD}/fonts" -Fex31CropNoLogo $ps
# b. make PS with outlined fonts:
#gs -q -sPAPERSIZE=a3 -dNOCACHE -dSAFER -dNOPAUSE -dBATCH -dNOPLATFONTS \
#  -sDEVICE=pswrite -sFONTPATH="${PWD}/fonts" -sOutputFile=$ps_outlined ex31CropNoLogo.eps
# c. make croppepd EPS:
#gs -q -dNOCACHE -dSAFER -dNOPAUSE -dBATCH -dEPSCrop -sDEVICE=epswrite \
#  -sOutputFile=$eps_outlined $ps_outlined
# d. make cropped PDF:
#ps2raster -P -A -Tf $ps_outlined

# uncomment to do conversation to PDF and PNG
# you will get a PDF with subsetted TrueType/PostScript fonts embedded
# which you can still edit with your favorite vector graphics editor
#export GS_FONTPATH="${PWD}/fonts"
#ps2raster -P -A -Tf $ps
#ps2raster -P -A -Tg -E110 $ps

# clean up
rm -f .gmtcommands* gmt.conf CUSTOM_font_info.d legend.txt ex31CropNoLogo.eps

exit 0
