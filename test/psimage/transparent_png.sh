#!/usr/bin/env bash

ps=transparent_png.ps

# Make several plots to test transparency
gmt psbasemap -R0/1/0/1 -JX7c -Y19c -B+glightblue+t"no option" -K -P > $ps
gmt psimage @warning.png -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -Y-9c -R -J -B+glightblue+t"-Gblack+t" -O -K >> $ps
gmt psimage @warning.png -Gblack+t -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gwhite+t" -O -K >> $ps
gmt psimage @warning.png -Gwhite+t -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X-8c -Y-9c -R -J -B+glightblue+t"-Gred+t" -O -K >> $ps
gmt psimage @warning.png -Gred+t -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gblue+t" -O -K >> $ps
gmt psimage @warning.png -Gblue+t -Dx0.5c/0.5c+jBL+w6c -O >> $ps
