#!/bin/bash
#               GMT ANIMATION 04
#               $Id: anim_04.sh,v 1.9 2011-06-07 20:12:06 guru Exp $
#
# Purpose:      Make DVD-res Quicktime movie of NY to Miami flight
# GMT progs:    gmtset, gmtmath, psbasemap, pstext, psxy, ps2raster
# Unix progs:   awk, mkdir, rm, mv, echo, qt_export, cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.
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
name=`basename $0 '.sh'`

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
		--PS_MEDIA=${px}ix${py}i -K > $$.ps
	psxy -R -J -O -K -W1p $$.path.d >> $$.ps
	pstext -R0/$px/0/$py -Jx1i -F+f14p,Helvetica-Bold+jTL -O >> $$.ps <<< "0 4.6 $ID"
	if [ $# -eq 0 ]; then
		mv $$.ps ../$name.ps
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
#	qt_export $$/anim_0_123456.tiff --video=h263,24,100, ${name}_movie.m4v
fi
# 4. Clean up temporary files
gmt_cleanup .gmt
