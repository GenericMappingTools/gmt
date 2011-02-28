#!/bin/bash
#               GMT ANIMATION 02
#               $Id: anim_02.sh,v 1.4 2011-02-28 00:58:03 remko Exp $
#
# Purpose:      Make web page with simple animated GIF of a DEM grid
# GMT progs:    gmtset, gmtmath, grdgradient, makecpt, grdimage psxy, ps2raster
# Unix progs:   awk, mkdir, rm, mv, echo, convert, cat
#
# 1. Initialization
# 1a) Assign movie parameters
. ../functions.sh
. gmt_shell_functions.sh
width=3.5i
height=4.15i
dpi=72
n_frames=36
TDIR=../../tutorial
name=../`basename $0 '.sh'`
# 1b) setup
del_angle=`gmtmath -Q 360 $n_frames DIV =`
makecpt -Crainbow -T500/4500/500 -Z > $$.cpt
gmtset DOTS_PR_INCH $dpi
R=`gmt_get_gridregion $TDIR/us.nc`
# 2. Main loop
mkdir -p $$
frame=0
while [ $frame -lt $n_frames ]; do
	# Create file name using a name_##.tif format
	file=`gmt_set_framename $name $frame`
	angle=`gmtmath -Q $frame $del_angle MUL =`
	dir=`gmtmath -Q $angle 180 ADD =`
	grdgradient $TDIR/us.nc -A$angle -Nt2 -M -G$$.us_int.nc
	grdimage $TDIR/us.nc -I$$.us_int.nc -JM3i -P -K -C$$.cpt -B1WSne -X0.35i -Y0.3i \
	--PAPER_MEDIA=Custom_${width}x${height} --ANNOT_FONT_SIZE=+9p > $$.ps
	echo 256.25 35.6 | psxy -R$R -J -O -K -Sc0.8i -Gwhite -Wthin >> $$.ps
	echo 256.25 35.6 $dir 0.37 | psxy -R$R -J -O -Sv0.02i/0.05i/0.05i -Gred -Wthin >> $$.ps
	if [ $# -eq 0 ]; then
		mv $$.ps $name.ps
		gmt_cleanup .gmt
		gmt_abort "$0: First frame plotted to $name.ps"
	fi
#	RIP to TIFF at specified dpi
	ps2raster -E$dpi -Tt $$.ps
	mv -f $$.tif $$/$file.tif
	echo "Frame $file completed"
	frame=`gmt_set_framenext $frame`
done
# 3. Create animated GIF file and HTML for web page
convert -delay 10 -loop 0 $$/*.tif $name.gif
cat << END > $name.html
<HTML>
<TITLE>GMT shading: A tool for feature detection</TITLE>
<BODY bgcolor="#ffffff">
<CENTER>
<H1>GMT shading: A tool for feature detection</H1>
<IMG src="$name.gif">
</CENTER>
<HR>
We make illuminated images of topography from a section of Colorado and
vary the azimuth of the illumination (see arrow).  As the light-source sweeps around
the area over 360 degrees we notice that different features in the data
become hightlighted.  This is because the illumination is based on data
gradients and such derivatives will high-light short-wavelength signal.
Again, our animation uses Imagemagick's convert tool to make an animated GIF file
with a 0.1 second pause between the 36 frames.
<HR>
<I>$name.sh: Created by $USER on `date`</I>
</BODY>
</HTML>
END
# 4. Clean up temporary files
gmtset DOTS_PR_INCH 300
gmt_cleanup .gmt
