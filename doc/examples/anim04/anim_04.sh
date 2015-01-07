#!/bin/bash
#               GMT ANIMATION 04
#               $Id$
#
# Purpose:      Make DVD-res movie of NY to Miami flight
# GMT progs:    gmt gmtset, gmt gmtmath, gmt psbasemap, gmt pstext, gmt psxy, gmt psconvert
# Unix progs:   awk, mkdir, rm, mv, echo, qt_export, cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.
#
# 1. Initialization
# 1a) Assign movie parameters
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
name=anim_04
ps=${name}.ps

# Set up flight path
gmt project -C-73.8333/40.75 -E-80.133/25.75 -G5 -Q > $$.path.d
frame=0
mkdir -p $$
gmt grdgradient USEast_Coast.nc -A90 -Nt1 -Gint_$$.nc
gmt makecpt -Cglobe -Z > globe_$$.cpt
function make_frame () {
	local frame file ID lon lat dist
	frame=$1; lon=$2; lat=$3; dist=$4
	file=`gmt_set_framename ${name} ${frame}`
	ID=`echo ${frame} | $AWK '{printf "%04d\n", $1}'`
	gmt grdimage -JG${lon}/${lat}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+ \
		${REGION} -P -Y0.1i -X0.1i USEast_Coast.nc -Iint_$$.nc -Cglobe_$$.cpt \
		--PS_MEDIA=${px}ix${py}i -K > ${file}_$$.ps
	gmt psxy -JG${lon}/${lat}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+ \
		${REGION} -O -K -W1p $$.path.d >> ${file}_$$.ps
	gmt pstext -R0/${px}/0/${py} -Jx1i -F+f14p,Helvetica-Bold+jTL -O >> ${file}_$$.ps <<< "0 4.6 ${ID}"
	[[ ${frame} -eq 0 ]] && cp ${file}_$$.ps ${ps}
	if [ $# -eq 0 ]; then
		gmt_cleanup .gmt
		gmt_abort "${0}: First frame plotted to ${name}.ps"
	fi
	gmt psconvert ${file}_$$.ps -Tt -E${dpi}
	mv ${file}_$$.tif $$/${file}.tif
	rm -f ${file}_$$.ps
	echo "Frame ${file} completed"
}
n_jobs=0
max_jobs=$(getconf _NPROCESSORS_ONLN || echo 1)
while read lon lat dist; do
	make_frame ${frame} ${lon} ${lat} ${dist} &
	((++n_jobs))
	frame=`gmt_set_framenext ${frame}`
	if [ ${n_jobs} -ge ${max_jobs} ]; then
		wait
		n_jobs=0
	fi
done < $$.path.d
wait

file=`gmt_set_framename ${name} 0`

echo "Made ${frame} frames at 720x480 pixels"
# GIF animate every 20th frame
${GRAPHICSMAGICK-gm} convert -delay 40 -loop 0 +dither $$/${name}_*[02468]0.tif ${name}.gif
if type -ft ${FFMPEG-ffmpeg} >/dev/null 2>&1 ; then
	# create H.264 video at 25fps
	echo "Creating H.264 video"
	${FFMPEG:-ffmpeg} -loglevel warning -y -f image2 -r 25 -i $$/${name}_%6d.tif \
		-vcodec libx264 -preset slower -crf 25 -pix_fmt yuv420p ${name}.mp4
	# create WebM video
	echo "Creating WebM video"
	${FFMPEG:-ffmpeg} -loglevel warning -y -f image2 -r 25 -i $$/${name}_%6d.tif \
		-vcodec libvpx -crf 10 -b:v 1.2M -pix_fmt yuv420p ${name}.webm
	# create Theora video
	echo "Creating Theora video"
	${FFMPEG:-ffmpeg} -loglevel warning -y -f image2 -r 25 -i $$/${name}_%6d.tif \
		-vcodec libtheora -q 4 -pix_fmt yuv420p ${name}.ogv
fi

# 4. Clean up temporary files
gmt_cleanup .gmt
