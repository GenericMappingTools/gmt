#!/bin/bash
#	$Id$
# Test makecpt -S options

ps=autocpt.ps
gmt makecpt @ship_15.txt -S -Cjet | gmt psscale -Dx0i/7i+w3i/0.2i+jML -P -K -By+l"-S" > $ps
gmt makecpt @ship_15.txt -S500 -Cjet | gmt psscale -Dx2i/7i+w3i/0.2i+jML -By+l"-S500" -O -K >> $ps
gmt makecpt @ship_15.txt -S500+d -Cjet | gmt psscale -Dx3.5i/7i+w3i/0.2i+jML -By+l"-S500+d" -O -K >> $ps
gmt makecpt @ship_15.txt -Sa3 -Cjet | gmt psscale -Dx5i/7i+w3i/0.2i+jML -By+l"-Sa3" -O -K >> $ps
gmt makecpt @ship_15.txt -Sm3 -Cjet | gmt psscale -Dx0i/2i+w3i/0.2i+jML -By+l"-Sm3" -O -K >> $ps
gmt makecpt @ship_15.txt -Sp3 -Cjet | gmt psscale -Dx2.5i/2i+w3i/0.2i+jML -By+l"-Sp3" -O -K >> $ps
gmt makecpt @ship_15.txt -Sq10/90 -Cjet | gmt psscale -Dx5i/2i+w3i/0.2i+jML -By+l"-Sq10/90" -O >> $ps
