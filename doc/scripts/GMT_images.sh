#!/bin/bash
#	$Id$
#
ps=GMT_images.ps
gmt psimage nsf1.jpg -W1.5i -R0/2/0/1 -JX5i/1.6i -B0 -P -K -DjML+o0.1i > $ps
#gmt psimage soest.eps -W2i -R -J -O -K -DjMR+o0.3i -F+p+gazure1 >> $ps
gmt psimage soest.eps -W2i -R -J -O -K -DjMR+o0.1i >> $ps
gmt psxy -R -J -O -T >> $ps
