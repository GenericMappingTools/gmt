REM		GMT EXAMPLE 31
REM		$Id$
REM
REM Purpose:	Illustrate usage of non-default fonts in PostScript
REM GMT progs:	gmtset, pscoast, psxy, pstext, pslegend
REM DOS calls:	del, echo, gawk
REM

echo GMT EXAMPLE 31
set ps=..\example_31.ps

REM create file CUSTOM_font_info.d in current working directory
REM and add PostScript font names of Linux Biolinum and Libertine

echo LinBiolinumO 0.700 0 > CUSTOM_font_info.d
echo LinBiolinumOI 0.700 0 >> CUSTOM_font_info.d
echo LinBiolinumOB 0.700 0 >> CUSTOM_font_info.d
echo LinLibertineOB 0.700 0 >> CUSTOM_font_info.d

REM common settings
gmtset FORMAT_GEO_MAP ddd:mm:ssF MAP_DEGREE_SYMBOL colon MAP_TITLE_OFFSET 20p MAP_GRID_CROSS_SIZE_PRIMARY 0.4c PS_LINE_JOIN round PS_CHAR_ENCODING ISO-8859-1 FONT LinBiolinumO FONT_TITLE 24p,LinLibertineOB MAP_ANNOT_OBLIQUE 42

REM map of countries
pscoast -Dl -R-7/31/64/66/r -JL15/50/40/60/16c -P -B10g10/5g5:."Europe\072 Countries and Capital Cities": -A250 -U"Example 31 in Cookbook" -Slightblue -Glightgreen -W0.25p -N1/1p,white -K > %ps%

REM mark capitals
psxy europe-capitals.csv -R -J -i0,1 -Sc0.15c -G196/80/80 -O -K >> %ps%

REM small EU cities
gawk "BEGIN {FS=\",\"} $4 !=\"\" && $4 <= 1000000 {print $1, $2}" europe-capitals.csv | psxy -R -J -Sc0.15c -W0.25p -O -K >> %ps%

REM big EU cities
gawk "BEGIN {FS=\",\"} $4 > 1000000 {print $1, $2}" europe-capitals.csv | psxy -R -J -Sc0.15c -W1.25p -O -K >> %ps%

REM label big EU cities
gawk "BEGIN {FS=\",\"} $4 > 1000000 {print $1, $2, $3}" europe-capitals.csv | pstext -R -J -F+f7p,LinBiolinumOI+jBL -Dj0.1c -Gwhite -C5%% -Qu -TO -O -K >> %ps%

REM construct legend
echo G -0.1c > legend.txt
echo H 10 LinBiolinumOB Population of the European Union capital cities >> legend.txt
echo G 0.15c >> legend.txt
echo N 2 >> legend.txt
echo 0 | gawk "{printf \"S 0.15c c 0.15c 196/80/80 0.25p 0.5c %%c 1 Million inhabitants\n\", 60}" >> legend.txt
echo 0 | gawk "{printf \"S 0.15c c 0.15c 196/80/80 1.25p 0.5c %%c 1 Million inhabitants\n\", 62}" >> legend.txt
echo N 1 >> legend.txt
echo G 0.15c >> legend.txt
echo L 8 LinBiolinumOB L Population in Millions >> legend.txt
echo N 6 >> legend.txt

REM append city names and population to legend
gawk "BEGIN {FS=\",\"; f=\"L 8 LinBiolinumO L\"} ($4 > 1000000) {printf \"%%s %%s:\n%%s %%.2f\n\", f, $3, f, $4/1e6}" europe-capitals.csv >> legend.txt

REM reduce annotation font size for legend
gmtset FONT_ANNOT_PRIMARY 8p

REM plot legend
pslegend -R -J -Gwhite -Dx7.9c/12.6c/8.0c/3.4c/BL -C0.3c/0.4c -L1.2 -F -O legend.txt >> %ps%

REM make a PostScript and a PDF file with outlined fonts
REM unfortunately ps2raster won't be able to crop that file correctly anymore
REM use Heiko Oberdiek's pdfcrop (http://code.google.com/p/pdfcrop2/) instead
REM or crop with ps2raster -A -Te before
REM
REM ps_outlined=example_outlined.ps
REM eps_outlined=example_outlined.eps
REM a. remove GMT logo and crop EPS:
REM ps2raster -P -Au -Te -C-sFONTPATH="${PWD}/fonts" -Fex31CropNoLogo %ps%
REM b. make PS with outlined fonts:
REM gs -q -sPAPERSIZE=a3 -dNOCACHE -dSAFER -dNOPAUSE -dBATCH -dNOPLATFONTS \
REM  -sDEVICE=pswrite -sFONTPATH="${PWD}/fonts" -sOutputFile=%ps_outlined% ex31CropNoLogo.eps
REM c. make croppepd EPS:
REM gs -q -dNOCACHE -dSAFER -dNOPAUSE -dBATCH -dEPSCrop -sDEVICE=epswrite \
REM  -sOutputFile=$eps_outlined %ps_outlined%
REM d. make cropped PDF:
REM ps2raster -P -A -Tf %ps_outlined%

REM uncomment to do conversation to PDF and PNG
REM you will get a PDF with subsetted TrueType/PostScript fonts embedded
REM which you can still edit with your favorite vector graphics editor
REM set GS_FONTPATH=fonts
REM ps2raster -P -A -Tf %ps%
REM ps2raster -P -A -Tg -E110 %ps%

REM clean up
del .gmt*
del CUSTOM_font_info.d
del legend.txt
rem del ex31CropNoLogo.eps
