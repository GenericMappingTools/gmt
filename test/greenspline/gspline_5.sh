#!/usr/bin/env bash
#

ps=gspline_5.ps

# Figures 2+3 in Wessel, P., and J. M. Becker (2008), Interpolation using a
# Generalized Green's Function for a Spherical Surface Spline in Tension,
# Geophys. J. Int., 174, 21-28.

data=$(gmt which -G @mag_obs_1990.txt)
# First find Parker's solution for no tension:
gmt greenspline -Rg -I1 $data -Sp -GFig_2_p0.nc
# Then repeat but use the wrong Oslo longitude to recreate Parker's original figure in his book
$AWK '{if ($1 == 10.45) {print 104.5, $2, $3} else {print $0}}' $data > tmp
gmt greenspline -Rg -I1 tmp -Sp -GFig_2_orig.nc
gmt pscoast -R0/360/0/90 -JA0/90/5i -P -Glightgray -K -B30 -Y5.5i --MAP_FRAME_WIDTH=0.025i \
	--FORMAT_GEO_MAP=dddF --FONT_ANNOT_PRIMARY=10p -X0.5 > $ps
echo 0 90 | gmt psxy -R -J -O -K -Sx0.1i -W0.5p >> $ps
gmt grdcontour -R Fig_2_p0.nc -J -O -K -Z+s0.001 -C5 -A10 -Gl195/0/0/90,0/90/295/0 >> $ps
gmt grdcontour -R Fig_2_orig.nc -J -O -K -Z+s0.001 -C5 -A10 -Wa0.75p,- -Wc0.25p,. -Gl160/0/270/80,270/80/340/0 >> $ps
gmt psxy -R -J -O -K $data -Sc0.1i -Gblack >> $ps
gmt psxy -R -J -O -K $data -Sc0.025i -Gwhite >> $ps
gmt psxy -R -J -O -K @mag_validate_1990.txt -Sc0.1i -Gwhite -W0.25p  >> $ps
gmt psxy -R -J -O -K @mag_validate_1990.txt -Sc0.025i -Gblack >> $ps
echo 104.50 59.92 | gmt psxy -R -J -O -K -Sc0.1i -Gwhite -W0.25p >> $ps
echo 104.50 59.92 | gmt psxy -R -J -O -K -Sx0.1i -W1p >> $ps
echo 104.50 59.92 -42 1.65 | gmt psxy -R -J -O -K -SV0.1i+e+h0.5 -W1p -Gblack >> $ps

# Repeat for Wessel&Becker's solution with t = 0.99

gmt greenspline -Rg -I1 $data -Sq0.99 -GFig_2_p5.nc

gmt pscoast -R0/360/0/90 -J -O -Glightgray -K -B30 -X2.5i -Y-5i --MAP_FRAME_WIDTH=0.025i \
	--FORMAT_GEO_MAP=dddF --FONT_ANNOT_PRIMARY=10 >> $ps
echo 0 90 | gmt psxy -R -J -O -K -Sx0.1i -W0.5p >> $ps
gmt grdcontour -R Fig_2_p5.nc -J -O -K -Z+s0.001 -C5 -A10 -Gl335/0/0/90,0/90/155/0 >> $ps
gmt psxy -R -J -O -K $data -Sc0.1i -Gblack >> $ps
gmt psxy -R -J -O -K $data -Sc0.025i -Gwhite >> $ps
gmt psxy -R -J -O -K @mag_validate_1990.txt -Sc0.1i -Gwhite -W0.25p  >> $ps
gmt psxy -R -J -O -K @mag_validate_1990.txt -Sc0.025i -Gblack >> $ps
gmt psxy -R -J -O -T >> $ps

