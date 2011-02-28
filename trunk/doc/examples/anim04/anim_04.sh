#!/bin/bash
#               GMT ANIMATION 04
#               $Id: anim_04.sh,v 1.7 2011-02-28 00:58:03 remko Exp $
#
# Purpose:      Make DVD-res MP4 movie of NY to Miami flight
# GMT progs:    gmtset, gmtmath, psbasemap, pstext, psxy, ps2raster
# Unix progs:   awk, mkdir, rm, mv, echo, qt_export, cat
#
# 1. Initialization
# 1a) Assign movie parameters
. ../functions.sh
. gmt_shell_functions.sh
REGION=-Rg
altitude=160.0
tilt=55
azimuth=210
twist=0
Width=36.0
Height=34.0
px=7.2
py=4.8
dpi=100
name=../`basename $0 '.sh'`

# Set up flight path
project -C-73.8333/40.75 -E-80.133/25.75 -G5 -Q > $$.path.d
frame=0
mkdir -p frames
grdgradient USEast_Coast.nc -A90 -Nt1 -G$$_int.nc
makecpt -Cglobe -Z > $$.cpt
while read lon lat dist; do
	file=`gmt_set_framename $name $frame`
	ID=`echo $frame | awk '{printf "%4.4d\n", $1}'`
	grdimage -JG${lon}/${lat}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+ \
		$REGION -P -Y0.1i -X0.1i USEast_Coast.nc -I$$_int.nc -C$$.cpt \
		--PAPER_MEDIA=Custom_${px}ix${py}i -K > $$.ps
	psxy -R -J -O -K -W1p $$.path.d >> $$.ps
	echo 0 4.6 14 0 1 TL $ID | pstext -R0/$px/0/$py -Jx1i -O >> $$.ps
	if [ $# -eq 0 ]; then
		mv $$.ps $name.ps
		gmt_cleanup .gmt
		gmt_abort "$0: First frame plotted to $name.ps"
	fi
	ps2raster $$.ps -Tt -E$dpi
	mv $$.tif frames/$file.tif
        echo "Frame $file completed"
	frame=`gmt_set_framenext $frame`
done < $$.path.d
if [ $# -eq 1 ]; then
	echo "anim_04.sh: Made $frame frames at 480x720 pixels placed in subdirectory frames"
	convert $$/anim_0_123456.tiff ${name}_movie.m4v
fi
# 4. Clean up temporary files
gmtset DOTS_PR_INCH 300
gmt_cleanup .gmt
