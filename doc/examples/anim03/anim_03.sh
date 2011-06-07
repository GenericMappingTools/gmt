#!/bin/bash
#               GMT ANIMATION 03
#               $Id: anim_03.sh,v 1.7 2011-06-07 20:12:06 guru Exp $
#
# Purpose:      Make web page with simple animated GIF of Iceland topo
# GMT progs:    gmtset, gmtmath, psbasemap, psxy, ps2raster
# Unix progs:   awk, mkdir, rm, mv, echo, convert, cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.
#
# 1. Initialization
# 1a) Assign movie parameters
. ../functions.sh
. gmt_shell_functions.sh
lon=-20
lat=65
dpi=100
x0=1.5
y0=0.75
px=4
py=2.5
el=35
az=0
name=`basename $0 '.sh'`
mkdir -p $$
gmtset PS_DPI $dpi
frame=0
grdclip -Sb0/-1 -G$$_above.nc Iceland.nc
grdgradient -fg -A45 -Nt1 $$_above.nc -G$$.nc
makecpt -Crelief -Z > $$.cpt
while [ $az -lt 360 ]; do
	file=`gmt_set_framename $name $frame`
	if [ $# -eq 0 ]; then	# If a single frame is requested we pick this view
		az=135
	fi
	grdview $$_above.nc -R-26/-12/63/67 -JM2.5 -C$$.cpt -Qi$dpi -B5g10/5g5 -P -X0.5i -Y0.5i \
		-p$az/${el}+w$lon/${lat}+v$x0/$y0 --PS_MEDIA=${px}ix${py}i > $$.ps
	if [ $# -eq 0 ]; then
		mv $$.ps ../$name.ps
		gmt_cleanup .gmt
		gmt_abort "$0: First frame plotted to $name.ps"
	fi
	ps2raster $$.ps -Tt -E$dpi
	mv $$.tif $$/$file.tif
	az=`expr $az + 5`
        echo "Frame $file completed"
	frame=`gmt_set_framenext $frame`
done
convert -delay 10 -loop 0 $$/*.tif $name.gif
cat << END > $name.html
<HTML>
<TITLE>GMT 3-D perspective of Iceland</TITLE>
<BODY bgcolor="#ffffff">
<CENTER>
<H1>GMT 3-D perspective of Iceland</H1>
<IMG src="$name.gif" border=1>
</CENTER>
<HR>
Here we show ETOPO2 topography of Iceland as we move the view
point around the island.
<I>$name.sh: Created by $USER on `date`</I>
</BODY>      
</HTML>
END
# 4. Clean up temporary files
gmt_cleanup .gmt
