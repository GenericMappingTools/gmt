#!/bin/bash
#	$Id$
# Test makecpt -S options

ps=autocpt.ps
gmt makecpt @ship_15.xyz -S -Cjet | gmt psscale -Dx0i/7i+w3i/0.2i+jML -P -K -By+l"-S" > $ps
gmt makecpt @ship_15.xyz -S500 -Cjet | gmt psscale -Dx2i/7i+w3i/0.2i+jML -By+l"-S500" -O -K >> $ps
gmt makecpt @ship_15.xyz -S500+d -Cjet | gmt psscale -Dx3.5i/7i+w3i/0.2i+jML -By+l"-S500+d" -O -K >> $ps
gmt makecpt @ship_15.xyz -Sa3 -Cjet | gmt psscale -Dx5i/7i+w3i/0.2i+jML -By+l"-Sa3" -O -K >> $ps
gmt makecpt @ship_15.xyz -Sm3 -Cjet | gmt psscale -Dx0i/2i+w3i/0.2i+jML -By+l"-Sm3" -O -K >> $ps
gmt makecpt @ship_15.xyz -Sp3 -Cjet | gmt psscale -Dx2.5i/2i+w3i/0.2i+jML -By+l"-Sp3" -O -K >> $ps
gmt makecpt @ship_15.xyz -Sq10/90 -Cjet | gmt psscale -Dx5i/2i+w3i/0.2i+jML -By+l"-Sq10/90" -O >> $ps
