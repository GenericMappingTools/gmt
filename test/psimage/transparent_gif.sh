#!/usr/bin/env bash
#

GDAL=`gmt grdconvert 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi

ps=transparent_gif.ps

# Make several plots to test transparency
gmt psbasemap -R0/1/0/1 -JX7c -Y19c -B+glightblue+t"no option" -K -P > $ps
gmt psimage @warning.gif -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -Y-9c -R -J -B+glightblue+t"-Gblack+t" -O -K >> $ps
gmt psimage @warning.gif -Gblack+t -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gwhite+t" -O -K >> $ps
gmt psimage @warning.gif -Gwhite+t -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X-8c -Y-9c -R -J -B+glightblue+t"-Gred+t" -O -K >> $ps
gmt psimage @warning.gif -Gred+t -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gblue+t" -O -K >> $ps
gmt psimage @warning.gif -Gblue+t -Dx0.5c/0.5c+jBL+w6c -O >> $ps
