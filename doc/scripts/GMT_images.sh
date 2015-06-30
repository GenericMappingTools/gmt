#!/bin/bash
#	$Id$
#
ps=GMT_images.ps
gmt psimage "${src:-.}"/nsf1.jpg -R0/2/0/1 -JX5i/1.6i -B0 -P -K -DjML+w1.5i+o0.1i > $ps
#gmt psimage soest.eps -W2i -R -J -O -K -DjMR+o0.3i -F+p+gazure1 >> $ps
gmt psimage "${src:-.}"/soest.eps -R -J -O -K -DjMR+o0.1i+w2i >> $ps
gmt psxy -R -J -O -T >> $ps
