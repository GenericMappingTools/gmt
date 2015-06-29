#!/bin/bash
#               GMT ANIMATION 03
#               $Id$
#
# Purpose:      Make web page with simple animated GIF of Iceland topo
# GMT progs:    gmt gmtset, gmt gmtmath, gmt psxy, gmt psconvert
# Unix progs:   awk, mkdir, rm, mv, echo, convert, cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.
#
# 1. Initialization
# 1a) Assign movie parameters
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
name=anim_03
ps=${name}.ps
mkdir -p $$
frame=0
gmt grdclip -Sb0/-1 -G$${_above}.nc Iceland.nc
gmt grdgradient -fg -A45 -Nt1 $${_above}.nc -G$$.nc
gmt makecpt -Crelief -T-2000/2000/20 > $$.cpt
while [ ${az} -lt 360 ]; do
	file=`gmt_set_framename ${name} ${frame}`
	if [ $# -eq 0 ]; then	# If a single frame is requested we pick this view
		az=135
	fi
	gmt grdview $${_above}.nc -R-26/-12/63/67 -JM2.5i -C$$.cpt -Qi${dpi} -Bx5g10 -By5g5 -P -X0.5i -Y0.5i \
		-p${az}/${el}+w${lon}/${lat}+v${x0}/${y0} --PS_MEDIA=${px}ix${py}i > $$.ps
	[[ ${az} -eq 135 ]] && cp $$.ps ${ps}
	if [ $# -eq 0 ]; then
		gmt_cleanup .gmt
		gmt_abort "${0}: First frame plotted to ${name}.ps"
	fi
	gmt psconvert $$.ps -Tt -E${dpi}
	mv $$.tif $$/${file}.tif
	az=`expr ${az} + 5`
        echo "Frame ${file} completed"
	frame=`gmt_set_framenext ${frame}`
done
${GRAPHICSMAGICK-gm} convert -delay 10 -loop 0 +dither $$/${name_}*.tif ${name}.gif
cat << END > ${name}.html
<HTML>
<TITLE>GMT 3-D perspective of Iceland</TITLE>
<BODY bgcolor="#ffffff">
<CENTER>
<H1>GMT 3-D perspective of Iceland</H1>
<IMG src="${name}.gif" border=1>
</CENTER>
<HR>
Here we show ETOPO2 topography of Iceland as we move the view
point around the island.
<I>${name}.sh: Created by ${USER} on `date`</I>
</BODY>      
</HTML>
END
# 4. Clean up temporary files
gmt_cleanup .gmt
