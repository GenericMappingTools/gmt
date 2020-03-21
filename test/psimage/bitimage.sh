#!/usr/bin/env bash
#

ps=bitimage.ps

# Raw, plus just change fore or background
gmt psimage @vader1.png -P -Dx0/0+w2i -F+pfaint -K > $ps
gmt psimage @vader1.png -Dx2.25i/0+w2i -F+pfaint -Gred+b -O -K >> $ps
gmt psimage @vader1.png -Dx4.5i/0+w2i -F+pfaint -Gred+f -O -K >> $ps

# Change both,then set transparent back or foreground
# The middle will show all white in PS but white and transparent in PNG
gmt psimage @vader1.png -Dx0i/2i+w2i -F+pfaint -Gred+f -Gyellow+b -O -K >> $ps
gmt psimage @vader1.png -Dx2.25i/2i+w2i -F+pfaint -G+b -O -K >> $ps
gmt psimage @vader1.png -Dx4.5i/2i+w2i -F+pfaint -G+f -O -K >> $ps

# Finally, mix transparency and change of color
gmt psimage @vader1.png -Dx0i/4i+w2i -F+pfaint -G+f -Gyellow+b -O -K >> $ps
gmt psimage @vader1.png -Dx2.25i/4i+w2i -F+pfaint -G+b -Gred+f -O -K >> $ps
gmt psimage @vader1.png -Dx4.5i/4i+w2i -F+pfaint -Gwhite+b -Gblack+f -O >> $ps
