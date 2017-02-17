#!/bin/bash
#               GMT ANIMATION 05
#               $Id$
#
# Purpose:      Make web page with simple animated GIF of gridding
# GMT modules:  grdcontour, greenspline, psxy, pstext, psconvert
# Unix progs:   mkdir, rm, mv, echo, convert, cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.
#
# 1. Initialization
# 1a) Assign movie parameters
. gmt_shell_functions.sh
n_eigen=`gmt_get_ndatarecords ${src:-.}/table_5.11`
dpi=100
name=anim_05
ps=${name}.ps
mkdir -p $$
gmt makecpt -Cpolar -T-25/25 > t.cpt
frame=0
let k=1
while [ $k -le $n_eigen ]; do
	file=`gmt_set_framename ${name} ${frame}`
	gmt greenspline ${src:-.}/table_5.11 -R0/6.5/0/6.5 -I0.05 -Sc -Gt.nc -D1 -Cn${k} -Emisfit.txt 2> /dev/null
	gmt grdcontour t.nc -C25 -A50 -Baf -BWSnE -JX4i -P -K -Gl3.6/6.5/4.05/0.75 -X0.25i -Y0.8i --PS_MEDIA=4.5ix5.0i > $$.ps
	gmt psxy misfit.txt -R -J -O -K -Ct.cpt -Sc0.15c -Wfaint -i0,1,4 >> $$.ps
	echo $k | gmt pstext -R -J -O -K -F+cTR+jTR+f18p -Dj0.1i >> $$.ps
	gmt psscale -R -J -O -Ct.cpt -DJBC+w3.4i/0.1i+h+jTC+o-0.2i/0.4i+e -Bxaf -By+l"misfit" >> $$.ps
	[[ ${frame} -eq 0 ]] && cp $$.ps ${ps}
	if [ $# -eq 0 ]; then
		gmt_cleanup .gmt
		gmt_abort "${0}: First frame plotted to ${name}.ps"
	fi
	gmt psconvert $$.ps -Tt -E${dpi}
	mv $$.tif $$/${file}.tif
	let k=k+1
        echo "Frame ${frame} completed"
	frame=`gmt_set_framenext ${frame}`
done
${GRAPHICSMAGICK-gm} convert -delay 10 -loop 0 +dither $$/${name_}*.tif ${name}.gif
cat << END > ${name}.html
<HTML>
<TITLE>GMT Spline gridding as function of number of eigenvalues</TITLE>
<BODY bgcolor="#ffffff">
<CENTER>
<H1>GMT Spline gridding as function of number of eigenvalues</H1>
<IMG src="${name}.gif" border=1>
</CENTER>
<HR>
Here we show how gridding with splines are affected by the number of eigenvalues
used in the construction of the final solution.
<I>${name}.sh: Created by ${USER} on `date`</I>
</BODY>      
</HTML>
END
# 4. Clean up temporary files
gmt_cleanup .gmt
