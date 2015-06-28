#!/bin/bash
#	$Id: GMT_anchor.sh 14405 2015-06-26 23:02:31Z pwessel $
#
ps=GMT_images.ps
gmt psimage nsf1.jpg -W2i -R0/2/0/1 -JX5i/2.1i -B0 -P -K -DjML+o0.1i > $ps
gmt psimage nsf1.jpg -W1.5i -R -J -O -K -DjMR+o0.3i -F+p+gazure1 >> $ps
gmt psxy -R -J -O -T >> $ps
