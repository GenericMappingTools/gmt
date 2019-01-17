#!/usr/bin/env bash
#               GMT ANIMATION 02
#
# Purpose:      Make web page with simple animated GIF of a DEM grid
# GMT modules   gmtmath, makecpt, grdimage psxy, psconvert
# Unix progs:   awk, mkdir, rm, mv, echo, convert, cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.
# GMT CLASSIC mode only
#
# 1. Initialization
# 1a) Assign movie parameters
. gmt_shell_functions.sh
width=3.5i
height=4.15i
dpi=72
n_frames=36
name=anim_02
ps=${name}.ps
# 1b) setup
del_angle=`gmt math -Q 360 ${n_frames} DIV =`
gmt makecpt -Crainbow -T500/4500 > $$.cpt
# 2. Main loop
mkdir -p $$
frame=0
while [ ${frame} -lt ${n_frames} ]; do
	# Create file name using a name_##.tif format
	file=`gmt_set_framename ${name} ${frame}`
	angle=`gmt math -Q ${frame} ${del_angle} MUL =`
	dir=`gmt math -Q ${angle} 180 ADD =`
	gmt grdimage @tut_relief.nc -I+a${angle}+nt2 -JM3i -P -K -C$$.cpt -BWSne -B1 -X0.35i -Y0.3i \
	--PS_MEDIA=${width}x${height} --FONT_ANNOT_PRIMARY=9p > $$.ps
	gmt psxy -R@tut_relief.nc -J -O -K -Sc0.8i -Gwhite -Wthin >> $$.ps <<< "256.25 35.6"
	gmt psxy -R@tut_relief.nc -J -O -Sv0.1i+e -Gred -Wthick >> $$.ps <<< "256.25 35.6 ${dir} 0.37"
	[[ ${frame} -eq 0 ]] && cp $$.ps ${ps}
	if [ $# -eq 0 ]; then
		gmt_cleanup .gmt
		gmt_abort "${0}: First frame plotted to ${name}.ps"
	fi
#	RIP to TIFF at specified dpi
	gmt psconvert -E${dpi} -Tt $$.ps
	mv -f $$.tif $$/${file}.tif
	echo "Frame ${file} completed"
	frame=`gmt_set_framenext ${frame}`
done
# 3. Create animated GIF file and HTML for web page
${GRAPHICSMAGICK-gm} convert -delay 10 -loop 0 $$/${name_}*.tif ${name}.gif
cat << END > ${name}.html
<HTML>
<TITLE>GMT shading: A tool for feature detection</TITLE>
<BODY bgcolor="#ffffff">
<CENTER>
<H1>GMT shading: A tool for feature detection</H1>
<IMG src="${name}.gif">
</CENTER>
<HR>
We make illuminated images of topography from a section of Colorado and
vary the azimuth of the illumination (see arrow).  As the light-source sweeps around
the area over 360 degrees we notice that different features in the data
become highlighted.  This is because the illumination is based on data
gradients and such derivatives will high-light short-wavelength signal.
Again, our animation uses GraphicsMagick's convert tool to make an animated GIF file
with a 0.1 second pause between the 36 frames.
<HR>
<I>${name}.sh: Created by ${USER} on `date`</I>
</BODY>
</HTML>
END
# 4. Clean up temporary files
gmt_cleanup .gmt
