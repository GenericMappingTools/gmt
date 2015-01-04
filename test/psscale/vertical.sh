#!/bin/bash
# Testing psscale -F option etc.
ps=vertical.ps
gmt set FONT_ANNOT_PRIMARY 12p FONT_LABEL 16p MAP_LABEL_OFFSET 8p
gmt makecpt -T-200/1000/100 -Crainbow > t.cpt
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxa200f -P -F+p1p+glightgray -K > $ps
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxa200f+l"topography" -By+lkm -F+p1p+i -O -K -X1.1i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxa200f+l"topography" -By+lkm -F+p1p -O -K -X1.4i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxa200f+l"topography" -By+lkm -F+p1p -A -O -K -X2.3i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxa200f+l"topography" -By+lkm -F+p1p -Aa -O -K -X1.1i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxa200f+l"topography" -By+lkm -F+p1p -Al -O -K -X-5.9i -Y4.8i >> $ps
gmt makecpt -T-20/10/1 -Crainbow > t.cpt
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxaf+l"topography" -By+lkm -F+p1p+r -Al -O -K -X1.3i >> $ps
gmt set FONT_ANNOT_PRIMARY 16p FONT_LABEL 24p MAP_LABEL_OFFSET 12p
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxaf+l"topography" -By+lmiles -F+p1p+r -Al -O -K -X1.4i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxaf+l"topography" -F+p1p+glightgray+s -Al -O -K -X1.7i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i/4i/0.2i -Bxaf+l"topography" -F+p1p+glightgray+s -Ac -O -X1.1i >> $ps
