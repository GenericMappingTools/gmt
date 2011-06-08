#!/bin/bash
#               GMT ANIMATION 01
#               $Id: anim_01.sh,v 1.7 2011-06-08 01:33:12 guru Exp $
#
# Purpose:      Make web page with simple animated GIF of sine function
# GMT progs:    gmtset, gmtmath, psbasemap, pstext, psxy, ps2raster
# Unix progs:   printf, mkdir, rm, mv, echo, convert, cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.
#
# 1. Initialization
# 1a) Assign movie parameters
. ../functions.sh
. gmt_shell_functions.sh
width=4i
height=2i
dpi=125
n_frames=18
name=`basename $0 '.sh'`
# 1b) Do frame-independent calculations and setup
angle_step=`gmtmath -Q 360 $n_frames DIV =`
angle_inc=`gmtmath -Q $angle_step 10 DIV =`
psbasemap -R0/360/-1.2/1.6 -JX3.5i/1.65i -P -K -X0.35i -Y0.25i \
	-Ba90g90f30:,-\\312:/a0.5f0.1g1WSne -Glightgreen \
	--PS_MEDIA=${width}x${height} --FONT_ANNOT_PRIMARY=9p > $$.map.ps
# 2. Main frame loop
mkdir -p $$
frame=0
while [ $frame -le $n_frames ]; do
	# Create file name using a name_##.tif format
	file=`gmt_set_framename $name $frame`
	cp -f $$.map.ps $$.ps
	angle=`gmtmath -Q $frame $angle_step MUL =`
	if [ $frame -gt 0 ]; then	# First plot has no curves
#		Plot smooth blue curve and dark red dots at all angle steps so far
		gmtmath -T0/$angle/$angle_inc T SIND = $$.sin.d
		psxy -R -J -O -K -W1p,blue $$.sin.d >> $$.ps
		gmtmath -T0/$angle/$angle_step T SIND = $$.sin.d
		psxy -R -J -O -K -Sc0.1i -Gdarkred $$.sin.d >> $$.ps
	fi
	#	Plot red dot at current angle and annotate
	sin=`gmtmath -Q $angle SIND =`
	psxy -R -J -O -K -Sc0.1i -Gred >> $$.ps <<< "$angle $sin"
	printf "0 1.6 a = %3.3d" $angle | pstext -R -J -F+f14p,Helvetica-Bold+jTL -O -K \
		-N -Dj0.1i/0.05i >> $$.ps
	psxy -R -J -O -T >> $$.ps
	if [ $# -eq 0 ]; then
		mv $$.ps ../$name.ps
		gmt_cleanup .gmt
		gmt_abort "$0: First frame plotted to $name.ps"
	fi
#	RIP to TIFF at specified dpi
	ps2raster -E$dpi -Tt $$.ps
	mv -f $$.tif $$/$file.tif
	echo "Frame $file completed"
	frame=`gmt_set_framenext $frame`
done
cp $$.ps t.ps
# 3. Create animated GIF file and HTML for web page
convert -delay 20 -loop 0 $$/*.tif $name.gif
cat << END > $name.html
<HTML>
<TITLE>GMT Trigonometry: The sine movie</TITLE>
<BODY bgcolor="#ffffff">
<CENTER>
<H1>GMT Trigonometry: The sine movie</H1>
<IMG src="$name.gif">
</CENTER>
<HR>
We demonstrate how the sine function <I>y = sin(a)</I> varies with <I>a</I> over
the full 360-degree interval.  We plot a bright red circle at each
new angle, letting previous circles turn dark red.  The underlying
sine curve is sampled at 10 times the frame sampling rate in order to reproduce
a smooth curve.  Our animation uses Imagemagick's convert tool to make an animated GIF file
with a 0.2 second pause between frames, set to repeat forever.
<HR>
<I>$name.sh: Created by $USER on `date`</I>
</BODY>
</HTML>
END
# 4. Clean up temporary files
gmt_cleanup .gmt
