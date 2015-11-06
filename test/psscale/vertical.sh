#!/bin/bash
# Testing psscale -F option etc.
ps=vertical.ps
gmt set FONT_ANNOT_PRIMARY 12p FONT_LABEL 16p MAP_LABEL_OFFSET 8p
gmt makecpt -T-200/1000/100 -Crainbow > t.cpt
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML -Bxa200f -P -F+p1p+glightgray -K > $ps
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML -Bxa200f+l"topography" -By+lkm -F+p1p+i -O -K -X1.1i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML -Bxa200f+l"topography" -By+lkm -F+p1p -O -K -X1.4i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML+m -Bxa200f+l"topography" -By+lkm -F+p1p -O -K -X2.3i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML+ma -Bxa200f+l"topography" -By+lkm -F+p1p -O -K -X1.1i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML+ml -Bxa200f+l"topography" -By+lkm -F+p1p -O -K -X-5.9i -Y4.8i >> $ps
gmt makecpt -T-20/10/1 -Crainbow > t.cpt
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML+ml -Bxaf+l"topography" -By+lkm -F+p1p+r -O -K -X1.3i >> $ps
gmt set FONT_ANNOT_PRIMARY 16p FONT_LABEL 24p MAP_LABEL_OFFSET 12p
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML+ml -Bxaf+l"topography" -By+lmiles -F+p1p+r -O -K -X1.4i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML+ml -Bxaf+l"topography" -F+p1p+glightgray+s -O -K -X1.5i >> $ps
gmt psscale -Ct.cpt -Dx0i/2i+w4i/0.2i+jML+mc -Bxaf+l"topography" -F+p1p+glightgray+s -O -X1.1i >> $ps
