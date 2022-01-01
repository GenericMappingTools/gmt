#!/usr/bin/env bash
#
# Movie of seismic waveforms from 2020-10-06 Alaska earthquake
# Epicenter 54.851N 159.851W
# Depth 31.0 km
# UTC time 2020-10-06 05:54:50
# Recorded at station Dolgoi Island, Mount Dutton, Alaska
#
# USGS event page: https://earthquake.usgs.gov/earthquakes/eventpage/us6000c5zm/executive
#
# DEM:  @earth_relief_06m
# Data: @waveform_AV.DOL.txt
#
# The data file @waveform_AV.DOL.txt contains 4 columns:
# UTC times, and three-component movement in east-west, north-south and up-down directions
#
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/eq_-2nYYf-s
# The 66-sec, 1561 frame movie took ~15 minutes to render on a 24-core MacPro 2013.

# 0. Create an include file with key parameters
cat << 'EOF' > inc.sh
TIME_RANGE=2020-10-06T05:55:10/2020-10-06T05:56:15
PLOT_DOMAIN=2020-10-06T05:55:10/2020-10-06T05:56:15/-2.6/2.8
DPI=200i
SPLINE=linear
EOF
# 1. Create a dense point-file from the waveform data
cat << 'EOF' > pre.sh
gmt begin
	# 1a. Create dense point-files from the waveform data components so we can animate the lines
	gmt events @waveform_AV.DOL.txt -R${PLOT_DOMAIN} -JX20cT/3.5c -Ar${DPI} -i0,1+s1e-6,0 -f2T --GMT_INTERPOLANT=${SPLINE} > E.txt
	gmt events @waveform_AV.DOL.txt -R${PLOT_DOMAIN} -JX20cT/3.5c -Ar${DPI} -i0,2+s1e-6,0 -f2T --GMT_INTERPOLANT=${SPLINE} > N.txt
	gmt events @waveform_AV.DOL.txt -R${PLOT_DOMAIN} -JX20cT/3.5c -Ar${DPI} -i0,3+s1e-6,0 -f2T --GMT_INTERPOLANT=${SPLINE} > Z.txt
	# 1b. Setup the desired output times (24 frames per second)
	gmt math -T${TIME_RANGE}/24+i -o0 T --TIME_UNIT=s --FORMAT_CLOCK_OUT=hh:mm:ss.xxxxx = times.txt
	# 1c. Lay down the static part of the subplot
	gmt subplot begin 3x1 -Fs18.25c/3.5c -A -M0 -R${PLOT_DOMAIN} -X5.5c -Y0.75c
		gmt subplot set 0 -A"Z"
		gmt plot @waveform_AV.DOL.txt -Bpxafg -Bsxa1D -Bpyaf -BWsrt -W0.5p,darkgray -i0,3+s1e-6
		gmt subplot set 1 -A"N"
		gmt plot @waveform_AV.DOL.txt -Bpxafg -Bsxa1D -Bpyaf -BWsrt -W0.5p,darkgray -i0,2+s1e-6
		gmt subplot set 2 -A"E"
		gmt plot @waveform_AV.DOL.txt -Bpxafg -Bsxa1D -Bpyaf -BWSrt -W0.5p,darkgray -i0,1+s1e-6
	gmt subplot end
	# 1d. Create location map and metadata
	gmt text -R0/4.5/0/13 -Jx1c -F+j+f -B0 -X-5.25c -Y-0.5c <<- EOF
	2.25 12.5 CM 18p 2020-10-06
	2.25 11.7 CM 9p SE of Sand Point, Alaska
	2.25 11 CM 10p 54.851@.N, 159.851@.W
	2.25 10.5 CM 10p 31.0 km, Mw 5.9
	2.25 10.0 CM 10p 2020-10-06 05:54:50 UTC
	EOF
	gmt grdimage -R190W/130W/30/75 @earth_relief_06m -Ba30+f -JM3.5c -I+d -Cterra --MAP_FRAME_TYPE=plain -X0.75c -Y0.5c -BWNbr --MAP_ANNOT_OBLIQUE=lat_parallel --FONT_ANNOT_PRIMARY=9p
	echo 161.86W 55.15N | gmt plot -St0.2c -GBlack
	echo 159.851W 54.851N | gmt plot -Sa0.25c -Gred -Wfaint
	gmt meca -Sc2.5c -M -G110/168/255 -W0.5p -Yh+0.5c <<- EOF
	-159.851 54.851 40.5 245 29 93 61 61 88 1.048 18
	EOF
gmt end
EOF
# 2. Set up main movie script
cat << 'EOF' > main.sh
gmt begin
	gmt set FORMAT_DATE_MAP "o dd yyyy" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p TIME_INTERVAL_FRACTION 0.01 FORMAT_CLOCK_MAP hh:mm:ss
	gmt subplot begin 3x1 -Fs18.25c/3.5c -D -M0 -R${PLOT_DOMAIN} -X5.5c -Y0.75c
		gmt events Z.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T${MOVIE_COL0} -c
		printf "%s -2.6\n%s 2.8\n" ${MOVIE_COL0} ${MOVIE_COL0} | gmt plot -W0.5p
		gmt events N.txt -Sc0.5p -Ggreen -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T${MOVIE_COL0} -c
		printf "%s -2.6\n%s 2.8\n" ${MOVIE_COL0} ${MOVIE_COL0} | gmt plot -W0.5p
		gmt events E.txt -Sc0.5p -Gblue -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T${MOVIE_COL0} -c
		printf "%s -2.6\n%s 2.8\n" ${MOVIE_COL0} ${MOVIE_COL0} | gmt plot -W0.5p
	gmt subplot end
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Iinc.sh -Sbpre.sh -CHD -Ttimes.txt -Nanim13 -Lc0+jTR+o0.25/0.25c -Ls"Station: Dolgoi Island, Mount Dutton, Alaska"+jTL+o5.5c/0.25c -H8 -Pb+w0.4c+jBR -Fmp4 -V -W -Zs -M500,png --FORMAT_DATE_MAP=-
